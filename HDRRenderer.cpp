#include "DXUT.h"
#include "SDKmisc.h"
#include "HDRRenderer.h"

HDRRenderer::HDRRenderer(void):m_width(0), m_height(0)
{
	m_brightPassFactor = 2;
	m_downSampleFactor = 4;

	m_brightPassThreshold = 0.5f;

	m_gaussianMul = 0.5;
	m_gaussianVariance = 1.0;

	m_exposure = 0.5f;
	m_white = 1.5f;
	InitPtr();
}

HDRRenderer::~HDRRenderer(void)
{
	OnDestroy();
}

HRESULT HDRRenderer::OnCreate( IDirect3DDevice9* pd3dDevice, int width, int height )
{
	HRESULT hr;

	m_width  = width;
	m_height = height;

	// create bright-pass texture
	int w = width / m_brightPassFactor;
	int h = height/ m_brightPassFactor;
	D3DFORMAT imgFmt = D3DFMT_A16B16G16R16F;
	V_RETURN(pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &m_brightPassTex, NULL));
	V_RETURN(m_brightPassTex->GetSurfaceLevel(0, &m_brightPassSurf));

	// create luminance texture
	const int factor = 3;
	imgFmt = D3DFMT_G16R16F;
	for(w = width/factor, h = height/factor; w >=1 && h >= 1; w/=factor, h/=factor)
	{
		LPDIRECT3DTEXTURE9 tex;
		LPDIRECT3DSURFACE9 surf;
		V_RETURN(pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &tex, NULL));
		V_RETURN(tex->GetSurfaceLevel(0, &surf));

		m_luminanceTexList.push_back(tex);
		m_luminanceSurfList.push_back(surf);
	}

	// create blur texture
	w = width  / m_brightPassFactor / m_downSampleFactor;
	h = height / m_brightPassFactor / m_downSampleFactor;
	imgFmt = D3DFMT_A16B16G16R16F;
	V_RETURN(pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &m_downSampledTex, NULL));
	V_RETURN(m_downSampledTex->GetSurfaceLevel(0, &m_downSampledSurf));
	V_RETURN(pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &m_horiBlurTex, NULL));
	V_RETURN(m_horiBlurTex->GetSurfaceLevel(0, &m_horiBlurSurf));
	V_RETURN(pd3dDevice->CreateTexture(w, h, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &m_veriBlurTex, NULL));
	V_RETURN(m_veriBlurTex->GetSurfaceLevel(0, &m_veriBlurSurf));

	// create effects
	WCHAR str[MAX_PATH];
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"HDR.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &m_processEffect, NULL ) );

	return hr;
}

HRESULT HDRRenderer::OnDestroy()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE(m_brightPassSurf);
	SAFE_RELEASE(m_downSampledSurf);
	SAFE_RELEASE(m_horiBlurSurf);
	SAFE_RELEASE(m_veriBlurSurf);

	SAFE_RELEASE(m_brightPassTex);
	SAFE_RELEASE(m_downSampledTex);
	SAFE_RELEASE(m_horiBlurTex);
	SAFE_RELEASE(m_veriBlurTex);

	for (int i = 0; i < m_luminanceTexList.size(); ++i)
	{
		SAFE_RELEASE(m_luminanceSurfList[i]);
		SAFE_RELEASE(m_luminanceTexList[i]);
	}
	m_luminanceTexList.clear();
	m_luminanceSurfList.clear();

	SAFE_RELEASE(m_processEffect);
	return hr;
}

HRESULT HDRRenderer::SetDownSample( int w, int h )
{
	HRESULT hr;

	float sU = 1 / (float)w * 0.5f;
	float sV = 1 / (float)h * 0.5f;
	float sU2= 1 / (float)w * 1.5f;
	float sV2= 1 / (float)h * 1.5f;

	D3DXVECTOR4 offsets[16] = {
		D3DXVECTOR4( -sU,  sV, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU,  sV, 0.0f, 0.0f ),
		D3DXVECTOR4( -sU, -sV, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU, -sV, 0.0f, 0.0f ),

		D3DXVECTOR4( -sU2,  sV, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU2,  sV, 0.0f, 0.0f ),
		D3DXVECTOR4( -sU2, -sV, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU2, -sV, 0.0f, 0.0f ),

		D3DXVECTOR4( -sU,  sV2, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU,  sV2, 0.0f, 0.0f ),
		D3DXVECTOR4( -sU, -sV2, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU, -sV2, 0.0f, 0.0f ),

		D3DXVECTOR4( -sU2,  sV2, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU2,  sV2, 0.0f, 0.0f ),
		D3DXVECTOR4( -sU2, -sV2, 0.0f, 0.0f ),
		D3DXVECTOR4(  sU2, -sV2, 0.0f, 0.0f ),
	};

	D3DXHANDLE offsetHdl = m_processEffect->GetParameterBySemantic(0, "SampleOffset");
	V_RETURN(m_processEffect->SetVectorArray(offsetHdl, offsets, 16));

	return hr;
}

void HDRRenderer::ComputeKernel( int w, int h,
								float* hKernel, 
								float* vKernel,
								float* hOffset, 
								float* vOffset,
								int length)
{
	float sU = 1 / (float)w * 0.5f;
	float sV = 1 / (float)h * 0.5f;

	// Compute the offsets. We take 9 samples - 4 either side and one in the middle:
	//     i =  0,  1,  2,  3, 4,  5,  6,  7,  8
	//Offset = -4, -3, -2, -1, 0, +1, +2, +3, +4
	int offset = length / 2;
	for (int i = 0; i < length; ++i)
	{
		hOffset[i] = (i-offset)*sU;
		vOffset[i] = (i-offset)*sV;

		float x = float(i-offset) / offset;
		float y = float(i-offset) / offset;

		hKernel[i] = m_gaussianMul * ComputeGaussianValue(x, m_gaussianVariance);
		vKernel[i] = m_gaussianMul * ComputeGaussianValue(y, m_gaussianVariance);
	}
}

HRESULT HDRRenderer::SetBlurKernel( int w, int h )
{
	HRESULT hr;
	const int c = 17;
	float hKernel[c], vKernel[c], hOffset[c], vOffset[c];
	ComputeKernel(w, h, hKernel, vKernel, hOffset, vOffset, c);
	D3DXVECTOR4 hParam[c];
	for (int i = 0; i < c; ++i)
	{
		hParam[i] = D3DXVECTOR4(hOffset[i], hKernel[i], vOffset[i], vKernel[i]);
	}

	D3DXHANDLE hBlurHdl = m_processEffect->GetParameterBySemantic(0, "OffsetAndWeight");

	V_RETURN(m_processEffect->SetVectorArray(hBlurHdl, hParam, c));
	return hr;
}

HRESULT HDRRenderer::BrightPass( LPDIRECT3DTEXTURE9 oriTexture, IDirect3DDevice9* device )
{
	HRESULT hr;

	device->SetRenderTarget(0, m_brightPassSurf);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	D3DXHANDLE brightThresholdHdl = m_processEffect->GetParameterBySemantic(0, "BPThreshold");
	D3DXHANDLE srcTexHdl          = m_processEffect->GetParameterBySemantic(0, "SourceTex");
	D3DXHANDLE techHdl			  = m_processEffect->GetTechniqueByName("brightPassTech");
	V_RETURN(m_processEffect->SetFloat(brightThresholdHdl, m_brightPassThreshold));
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, oriTexture));
	V_RETURN(m_processEffect->SetTechnique(techHdl));
	SetDownSample(m_width, m_height);
	int w, h;
	GetTextureDim(m_brightPassTex, w,h);

	UINT nPass = 0;
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, w,h);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

	//D3DXSaveTextureToFile(L"composite.png", D3DXIFF_PNG, oriTexture, NULL);
	//D3DXSaveTextureToFile(L"bright.jpg", D3DXIFF_JPG, m_brightPassTex, NULL);
	return hr;
}

void HDRRenderer::InitPtr()
{
	m_brightPassTex = m_downSampledTex = m_horiBlurTex = m_veriBlurTex = NULL;
	m_brightPassSurf= m_downSampledSurf= m_horiBlurSurf= m_veriBlurSurf= NULL;

	m_processEffect = NULL;
}

void HDRRenderer::GetTextureDim( LPDIRECT3DTEXTURE9 tex, int&w, int& h )
{
	D3DSURFACE_DESC desc;
	tex->GetLevelDesc(0, &desc);

	w = desc.Width;
	h = desc.Height;
}

HRESULT HDRRenderer::LuminancePass( LPDIRECT3DTEXTURE9 oriTexture, IDirect3DDevice9* device )
{
	HRESULT hr;
	int w, h;

	device->SetRenderTarget(0, m_luminanceSurfList[0]);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	GetTextureDim(m_luminanceTexList[0], w, h);

	D3DXHANDLE srcTexHdl          = m_processEffect->GetParameterBySemantic(0, "SourceTex");
	D3DXHANDLE lumiTechHdl		  = m_processEffect->GetTechniqueByName("luminanceTech");
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, oriTexture));
	V_RETURN(m_processEffect->SetTechnique(lumiTechHdl));
	SetDownSample(m_width, m_height);

	UINT nPass = 0;
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, w, h);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

	//D3DXSaveTextureToFile(L"composite.png", D3DXIFF_PNG, oriTexture, NULL);
	//D3DXSaveTextureToFile(L"luminance.jpg", D3DXIFF_JPG, m_luminanceTexList[0], NULL);

	// down sample
	D3DXHANDLE downSampleTechHdl  = m_processEffect->GetTechniqueByName("downSampleTech");
	V_RETURN(m_processEffect->SetTechnique(downSampleTechHdl));
	for (int ithTex = 1; ithTex < m_luminanceTexList.size(); ++ithTex)
	{
		device->SetRenderTarget(0, m_luminanceSurfList[ithTex]);
		device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

		LPDIRECT3DTEXTURE9 srcTex = m_luminanceTexList[ithTex-1];
		LPDIRECT3DTEXTURE9 tarTex = m_luminanceTexList[ithTex];
		int srcW, srcH, tarW, tarH;
		GetTextureDim(srcTex, srcW, srcH);
		GetTextureDim(tarTex, tarW, tarH);
		SetDownSample(srcW, srcH);
		V_RETURN(m_processEffect->SetTexture(srcTexHdl, srcTex));

		UINT nPass = 0;
		m_processEffect->Begin(&nPass, 0);
		for (int ithPass = 0; ithPass < nPass; ++ithPass)
		{
			m_processEffect->BeginPass(ithPass);
			ScreenQuad::DrawFullScreen(device, tarW, tarH);
			m_processEffect->EndPass();
		}
		m_processEffect->End();

//  		WCHAR fileName[20];
//  		swprintf(fileName, L"downSample_%d.jpg", ithTex);
//  		D3DXSaveTextureToFile(fileName, D3DXIFF_JPG, m_luminanceTexList[ithTex], NULL);
	}
	return hr;
}

HRESULT HDRRenderer::BlurPass( LPDIRECT3DTEXTURE9 oriTexture,IDirect3DDevice9* device )
{
	HRESULT hr;

	// downsample bright pass result
	device->SetRenderTarget(0, m_downSampledSurf);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	D3DXHANDLE downSampleTechHdl  = m_processEffect->GetTechniqueByName("downSampleTech");
	D3DXHANDLE srcTexHdl          = m_processEffect->GetParameterBySemantic(0, "SourceTex");
	V_RETURN(m_processEffect->SetTechnique(downSampleTechHdl));
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, m_brightPassTex));
	int brightW, brightH, dsWidth, dsHeight;
	GetTextureDim(m_brightPassTex, brightW, brightH);
	GetTextureDim(m_downSampledTex, dsWidth, dsHeight);
	SetDownSample(brightW, brightH);

	UINT nPass = 0;
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, dsWidth, dsHeight);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

	//D3DXSaveTextureToFile(L"brightPassDownsample.jpg", D3DXIFF_JPG, m_downSampledTex, NULL);

	// blur
	SetBlurKernel(dsWidth, dsHeight);
	D3DXHANDLE hBlurTechHdl  = m_processEffect->GetTechniqueByName("horiBlurTech");
	D3DXHANDLE vBlurTechHdl  = m_processEffect->GetTechniqueByName("vertBlurTech");

	// h blur
	device->SetRenderTarget(0, m_horiBlurSurf);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	V_RETURN(m_processEffect->SetTechnique(hBlurTechHdl));
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, m_downSampledTex));		
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, dsWidth, dsHeight);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

	// v blur
	device->SetRenderTarget(0, m_veriBlurSurf);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	V_RETURN(m_processEffect->SetTechnique(vBlurTechHdl));
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, m_horiBlurTex));		
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, dsWidth, dsHeight);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

 	//D3DXSaveTextureToFile(L"hBlur.png", D3DXIFF_PNG, m_horiBlurTex, NULL);
 	//D3DXSaveTextureToFile(L"vBlur.png", D3DXIFF_PNG, m_veriBlurTex, NULL);
	return hr;
}



HRESULT HDRRenderer::Render(LPDIRECT3DTEXTURE9 oriTexture, 
							LPDIRECT3DSURFACE9 backBuffer,
							IDirect3DDevice9* device )
{
	HRESULT hr;
	DWORD zEnable, zWrite;
	device->GetRenderState(D3DRS_ZENABLE, &zEnable);
	device->GetRenderState(D3DRS_ZWRITEENABLE, &zWrite);
	device->SetRenderState(D3DRS_ZENABLE, false);
	device->SetRenderState(D3DRS_ZWRITEENABLE, false);

	V_RETURN(BrightPass(oriTexture, device));
	V_RETURN(LuminancePass(oriTexture, device));
	V_RETURN(BlurPass(oriTexture, device));
	V_RETURN(FinalPass(oriTexture, device, backBuffer));

	device->SetRenderState(D3DRS_ZENABLE, zEnable);
	device->SetRenderState(D3DRS_ZWRITEENABLE, zWrite);
	return hr;
}

HRESULT HDRRenderer::FinalPass( LPDIRECT3DTEXTURE9 oriTexture,IDirect3DDevice9* device, LPDIRECT3DSURFACE9 backBuffer )
{
	HRESULT hr ;

	// set render targets
	device->SetRenderTarget(0, backBuffer);
	device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	// prepare parameters
	D3DXHANDLE finalTechHdl		  = m_processEffect->GetTechniqueByName("finalTech");
	D3DXHANDLE srcTexHdl          = m_processEffect->GetParameterBySemantic(0, "SourceTex");
	D3DXHANDLE luminanceTexHdl    = m_processEffect->GetParameterBySemantic(0, "LuminanceTex");
	D3DXHANDLE blurTexHdl         = m_processEffect->GetParameterBySemantic(0, "BloomTex");
	D3DXHANDLE exposureHdl		  = m_processEffect->GetParameterBySemantic(0, "ExposureFactor");
	D3DXHANDLE whiteHdl		  = m_processEffect->GetParameterBySemantic(0, "WhiteFactor");
	V_RETURN(m_processEffect->SetTechnique(finalTechHdl));
	V_RETURN(m_processEffect->SetTexture(srcTexHdl, oriTexture));
	V_RETURN(m_processEffect->SetTexture(luminanceTexHdl, m_luminanceTexList.back()));
	V_RETURN(m_processEffect->SetTexture(blurTexHdl, m_veriBlurTex));
	V_RETURN(m_processEffect->SetFloat(exposureHdl, m_exposure));
	V_RETURN(m_processEffect->SetFloat(whiteHdl, m_white));

	UINT nPass = 0;
	m_processEffect->Begin(&nPass, 0);
	for (int i = 0; i < nPass; ++i)
	{
		m_processEffect->BeginPass(i);
		ScreenQuad::DrawFullScreen(device, m_width, m_height);
		m_processEffect->EndPass();
	}
	m_processEffect->End();

	return hr;
}

HRESULT HDRRenderer::OnReset( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr = S_OK;
	if( m_processEffect )
		V_RETURN( m_processEffect->OnResetDevice() );
	return hr;
}

HRESULT HDRRenderer::OnLost()
{
	HRESULT hr = S_OK;
	if( m_processEffect )
		m_processEffect->OnLostDevice();
	return hr;
}
