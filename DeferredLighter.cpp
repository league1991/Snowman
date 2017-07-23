#include "DXUT.h"
#include "DeferredLighter.h"
#include "SDKmisc.h"


DeferredLighter::~DeferredLighter(void)
{
}

DeferredLighter::DeferredLighter( CMeshLoader* meshloader, CFirstPersonCamera* camera, SkyBox* skybox ):
lightClr(0.8,0.8,1), lightDir(1,1,1)
{
	m_Camera = camera;
	m_MeshLoader = meshloader;
	m_skyBox = skybox;

	m_hAmbient = NULL;
	m_hDiffuse = NULL;
	m_hSpecular = NULL;
	m_hOpacity = NULL;
	m_hSpecularPower = NULL;
	m_hLightColor = NULL;
	m_hLightPosition = NULL;
	m_hCameraPosition = NULL;
	m_hTexture = NULL;
	m_hTime = NULL;
	m_hView = NULL;
	m_hWorldViewProjection = NULL;

	m_pBackBuffer = NULL;
	pTempSurface = NULL;

	m_pBackBuffer = NULL;
	pTempSurface = NULL;

	fTime = 0;
	fElapsedTime = 0;
}

HRESULT DeferredLighter::OnCreate( IDirect3DDevice9* pd3dDevice, int width, int height )
{
	HRESULT hr;
	WCHAR str[MAX_PATH];

	m_width = width;
	m_height = height;

	// Read the D3DX effect file and load the effect
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"geometryFx.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &m_pGeoEffect, NULL ) );
	m_hAmbient = m_pGeoEffect->GetParameterBySemantic( 0, "Ambient" );
	m_hDiffuse = m_pGeoEffect->GetParameterBySemantic( 0, "Diffuse" );
	m_hSpecular = m_pGeoEffect->GetParameterBySemantic( 0, "Specular" );
	m_hOpacity = m_pGeoEffect->GetParameterBySemantic( 0, "Opacity" );
	m_hSpecularPower = m_pGeoEffect->GetParameterBySemantic( 0, "SpecularPower" );
	m_hLightColor = m_pGeoEffect->GetParameterBySemantic( 0, "LightColor" );
	m_hLightPosition = m_pGeoEffect->GetParameterBySemantic( 0, "LightPosition" );
	m_hCameraPosition = m_pGeoEffect->GetParameterBySemantic( 0, "CameraPosition" );
	m_hTexture = m_pGeoEffect->GetParameterBySemantic( 0, "Texture" );
	m_hTime = m_pGeoEffect->GetParameterBySemantic( 0, "Time" );
	m_hView = m_pGeoEffect->GetParameterBySemantic( 0, "View" );
	m_hWorldViewProjection = m_pGeoEffect->GetParameterBySemantic( 0, "WorldViewProjection" );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"LightBufferFx.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &m_pLightEffect, NULL ) );

	V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"MeshFromOBJ.fx" ) );
	V_RETURN( D3DXCreateEffectFromFile( pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &m_pCompositeEffect, NULL ) );

	// init render target
	D3DFORMAT imgFmt = D3DFMT_A16B16G16R16F;
	for (int i = 0; i < TEX_TEXTURECOUNT; ++i)
	{
		V_RETURN(pd3dDevice->CreateTexture(m_width, m_height, 1, D3DUSAGE_RENDERTARGET, imgFmt, D3DPOOL_DEFAULT, &m_pRenderTexture[i], NULL));
		V_RETURN(m_pRenderTexture[i]->GetSurfaceLevel(0, &m_pRenderSurface[i]));
	}

	return hr;
}



HRESULT DeferredLighter::OnRender( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr = S_OK;

	// ----------------------------------------------------
	// Geometry Pass
	// ----------------------------------------------------
	V_RETURN(GeometryPass(pd3dDevice));

	// ----------------------------------------------------
	// Lighting Pass
	// ----------------------------------------------------
	V_RETURN(LightingPass(pd3dDevice));

	// ----------------------------------------------------
	// Composite Pass
	// ----------------------------------------------------		
	V_RETURN(CompositePass(pd3dDevice));

	return hr;
}

void DeferredLighter::RenderSubset( UINT iSubset, ID3DXEffect* effect )
{
	HRESULT hr;
	UINT iPass, cPasses;

	// Retrieve the ID3DXMesh pointer and current material from the MeshLoader helper
	ID3DXMesh* pMesh = m_MeshLoader->GetMesh();

	V( effect->Begin( &cPasses, 0 ) );
	for( iPass = 0; iPass < cPasses; iPass++ )
	{
		V( effect->BeginPass( iPass ) );

		// The effect interface queues up the changes and performs them 
		// with the CommitChanges call. You do not need to call CommitChanges if 
		// you are not setting any parameters between the BeginPass and EndPass.
		// V( effect->CommitChanges() );

		// Render the mesh with the applied technique
		V( pMesh->DrawSubset( iSubset ) );
		V( effect->EndPass() );
	}
	V( effect->End() );
}

void DeferredLighter::SetTime( double time, float elapsedTime )
{
	fTime = time;
	fElapsedTime = elapsedTime;
}

HRESULT DeferredLighter::OnResetDevice( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr = S_OK;
	if( m_pGeoEffect )
		V_RETURN( m_pGeoEffect->OnResetDevice() );
	if( m_pLightEffect )
		V_RETURN( m_pLightEffect->OnResetDevice() );
	if( m_pCompositeEffect )
		V_RETURN( m_pCompositeEffect->OnResetDevice() );

	// Store the correct technique handles for each material
	for( UINT i = 0; i < m_MeshLoader->GetNumMaterials(); i++ )
	{
		Material* pMaterial = m_MeshLoader->GetMaterial( i );

		const char* strTechnique = NULL;

		if( pMaterial->pDiffuseTex && pMaterial->bSpecular )
			strTechnique = "TexturedSpecular";
		else if( pMaterial->pDiffuseTex && !pMaterial->bSpecular )
			strTechnique = "TexturedNoSpecular";
		else if( !pMaterial->pDiffuseTex && pMaterial->bSpecular )
			strTechnique = "Specular";
		else if( !pMaterial->pDiffuseTex && !pMaterial->bSpecular )
			strTechnique = "NoSpecular";

		pMaterial->hTechnique = m_pGeoEffect->GetTechniqueByName( strTechnique );
	}
	return hr;
}

HRESULT DeferredLighter::OnDestroyDevice()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pGeoEffect );
	SAFE_RELEASE( m_pLightEffect);
	SAFE_RELEASE( m_pCompositeEffect);

	for (int i = 0; i < TEX_TEXTURECOUNT; ++i)
	{
		SAFE_RELEASE(m_pRenderSurface[i]);
		SAFE_RELEASE(m_pRenderTexture[i]);
	}
	return hr;
}

HRESULT DeferredLighter::OnLostDevice()
{
	HRESULT hr = S_OK;
	if( m_pGeoEffect )
		m_pGeoEffect->OnLostDevice();
	if( m_pLightEffect )
		m_pLightEffect->OnLostDevice();
	if( m_pCompositeEffect )
		m_pCompositeEffect->OnLostDevice();
	return hr;
}

LPDIRECT3DTEXTURE9 DeferredLighter::GetRenderTexture( TextureType tex )
{
	return m_pRenderTexture[tex];
}

HRESULT DeferredLighter::GeometryPass( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;

	mView = *m_Camera->GetViewMatrix();
	mProj = *m_Camera->GetProjMatrix();
	mWorldViewProjection = mView * mProj;

	hr = pd3dDevice->SetRenderTarget(0, m_pRenderSurface[TEX_NORMAL]);
	hr = pd3dDevice->SetRenderTarget(1, m_pRenderSurface[TEX_POS]);
	hr = pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);
	pd3dDevice->SetRenderState(D3DRS_ZENABLE, true);
	pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);

	// Update the effect's variables.
	V( m_pGeoEffect->SetMatrix( m_hWorldViewProjection, &mWorldViewProjection ) );
	V( m_pGeoEffect->SetMatrix( m_hView, &mView ) );
	V( m_pGeoEffect->SetFloat( m_hTime, ( float )fTime ) );
	V( m_pGeoEffect->SetValue( m_hCameraPosition, m_Camera->GetEyePt(), sizeof( D3DXVECTOR3 ) ) );

	// Iterate through subsets, changing material properties for each
	for( UINT iSubset = 0; iSubset < m_MeshLoader->GetNumMaterials(); iSubset++ )
	{
		// Set the lighting variables and texture for the current material
		Material* pMaterial = m_MeshLoader->GetMaterial( iSubset );
		V( m_pGeoEffect->SetValue( m_hAmbient, pMaterial->vAmbient, sizeof( D3DXVECTOR3 ) ) );
		V( m_pGeoEffect->SetValue( m_hDiffuse, pMaterial->vDiffuse, sizeof( D3DXVECTOR3 ) ) );
		V( m_pGeoEffect->SetValue( m_hSpecular, pMaterial->vSpecular, sizeof( D3DXVECTOR3 ) ) );
		V( m_pGeoEffect->SetTexture( m_hTexture, pMaterial->pDiffuseTex ) );
		V( m_pGeoEffect->SetFloat( m_hOpacity, pMaterial->fAlpha ) );
		V( m_pGeoEffect->SetInt( m_hSpecularPower, pMaterial->nShininess ) );
		// has normal map
		D3DXHANDLE techHdl;
		if (pMaterial->pNormalTex)
		{
			D3DXHANDLE normalTexHdl = m_pGeoEffect->GetParameterBySemantic(0, "NormalTexture");
			V( m_pGeoEffect->SetTexture( normalTexHdl, pMaterial->pNormalTex ) );
			techHdl = m_pGeoEffect->GetTechniqueByName("GeoTechWithNormal");
		}
		else
			techHdl = m_pGeoEffect->GetTechniqueByName("GeoTechWithoutNormal");
		m_pGeoEffect->SetTechnique(techHdl);
		RenderSubset( iSubset, m_pGeoEffect);
	}

	//D3DXSaveTextureToFile(L"normal.jpg", D3DXIFF_JPG, m_pRenderTexture[TEX_NORMAL], NULL);
	//D3DXSaveTextureToFile(L"position.jpg", D3DXIFF_JPG, m_pRenderTexture[TEX_POS], NULL);

	return hr;
}

HRESULT DeferredLighter::LightingPass( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;

	pd3dDevice->SetRenderTarget(0, m_pRenderSurface[TEX_LIGHT]);
	pd3dDevice->SetRenderTarget(1, NULL);
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	D3DXHANDLE lightClrHdl = m_pLightEffect->GetParameterBySemantic( 0, "LightColor" );
	D3DXHANDLE lightDirHdl = m_pLightEffect->GetParameterBySemantic( 0, "LightDirection" );
	D3DXHANDLE cameraPosHdl = m_pLightEffect->GetParameterBySemantic( 0, "CameraPosition" );
	D3DXHANDLE normalTexHdl = m_pLightEffect->GetParameterBySemantic( 0, "NormalTex" );
	D3DXHANDLE worldPosTexHdl = m_pLightEffect->GetParameterBySemantic( 0, "WorldPosTex" );
	lightClr = D3DXVECTOR3 (20,2,2);
	lightDir = D3DXVECTOR3 (1,1,0);
	m_pLightEffect->SetValue(lightClrHdl, &lightClr, sizeof(D3DXVECTOR3));
	m_pLightEffect->SetValue(lightDirHdl, &lightDir, sizeof(D3DXVECTOR3));
	m_pLightEffect->SetValue(cameraPosHdl, m_Camera->GetEyePt(), sizeof(D3DXVECTOR3));
	m_pLightEffect->SetTexture(normalTexHdl, m_pRenderTexture[TEX_NORMAL]);
	m_pLightEffect->SetTexture(worldPosTexHdl, m_pRenderTexture[TEX_POS]);  

	UINT nPass = 0;
	V( m_pLightEffect->Begin( &nPass, 0 ) );
	for(int i = 0; i < nPass; i++ )
	{
		V( m_pLightEffect->BeginPass( i ) );
		ScreenQuad::DrawFullScreen(pd3dDevice, m_width, m_height);
		V( m_pLightEffect->EndPass() );
	}
	V( m_pLightEffect->End() );
	//D3DXSaveTextureToFile(L"light.jpg", D3DXIFF_JPG, m_pRenderTexture[TEX_LIGHT], NULL);

	return hr;
}

HRESULT DeferredLighter::CompositePass( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr;
	pd3dDevice->SetRenderTarget(0, m_pRenderSurface[TEX_COMPOSITE]);
	pd3dDevice->SetRenderTarget(1, NULL);
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0);

	// Render sky box
	m_skyBox->SetMatrix(mView, mProj);
	m_skyBox->Render();

	float pixelOffset[2] = {1/float(m_width)*0.5f, 1/float(m_height)*0.5f};
	D3DXHANDLE worldViewProjHdl = m_pCompositeEffect->GetParameterBySemantic(0, "WorldViewProjection");
	D3DXHANDLE AmbientHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "Ambient" );
	D3DXHANDLE DiffuseHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "Diffuse" );
	D3DXHANDLE SpecularHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "Specular" );
	D3DXHANDLE SpecularPowerHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "SpecularPower" );
	D3DXHANDLE LightColorHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "LightColor" );
	D3DXHANDLE LightPositionHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "LightPosition" );
	D3DXHANDLE CameraPositionHdl = m_pCompositeEffect->GetParameterBySemantic( 0, "CameraPosition" );
	D3DXHANDLE lightTexHdl       = m_pCompositeEffect->GetParameterBySemantic(0, "LightTex");
	D3DXHANDLE compNormalTexHdl      = m_pCompositeEffect->GetParameterBySemantic(0, "NormalTex");
	D3DXHANDLE pixelOffsetHdl        = m_pCompositeEffect->GetParameterBySemantic(0, "PixelOffset");
	D3DXHANDLE compDiffuseTexHdl        = m_pCompositeEffect->GetParameterBySemantic(0, "ObjColorTex");
	D3DXHANDLE compSpecularTexHdl        = m_pCompositeEffect->GetParameterBySemantic(0, "ObjSpecTex");
	D3DXHANDLE compositeTech[4];
	compositeTech[0] = m_pCompositeEffect->GetTechniqueByName("NoDiffuseNoSpecular");
	compositeTech[1] = m_pCompositeEffect->GetTechniqueByName("DiffuseNoSpecular");
	compositeTech[2] = m_pCompositeEffect->GetTechniqueByName("NoDiffuseSpecular");
	compositeTech[3] = m_pCompositeEffect->GetTechniqueByName("DiffuseSpecular");
	lightClr = D3DXVECTOR3 (255,217,128) / 255.f * 2;
	V( m_pCompositeEffect->SetMatrix( worldViewProjHdl, &mWorldViewProjection ) );
	V( m_pCompositeEffect->SetValue( CameraPositionHdl, m_Camera->GetEyePt(), sizeof( D3DXVECTOR3 ) ) );
	V( m_pCompositeEffect->SetValue( LightColorHdl, &lightClr, sizeof( D3DXVECTOR3 ) ) );
	V( m_pCompositeEffect->SetValue( pixelOffsetHdl, pixelOffset, sizeof(float)*2 ) );
	m_pCompositeEffect->SetTexture(lightTexHdl, m_pRenderTexture[TEX_LIGHT]);
	m_pCompositeEffect->SetTexture(compNormalTexHdl, m_pRenderTexture[TEX_NORMAL]);

	D3DXVECTOR3 ambient = D3DXVECTOR3(0,0,0);//D3DXVECTOR3(66,90,119) / 255.0;
	V( m_pCompositeEffect->SetValue( AmbientHdl, ambient, sizeof( D3DXVECTOR3 ) ) );
	// Iterate through subsets, changing material properties for each
	for( UINT iSubset = 0; iSubset < m_MeshLoader->GetNumMaterials(); iSubset++ )
	{
		// Set the lighting variables and texture for the current material
		Material* pMaterial = m_MeshLoader->GetMaterial( iSubset );
		V( m_pCompositeEffect->SetValue( DiffuseHdl, pMaterial->vDiffuse, sizeof( D3DXVECTOR3 ) ) );
		V( m_pCompositeEffect->SetValue( SpecularHdl, pMaterial->vSpecular, sizeof( D3DXVECTOR3 ) ) );
		V( m_pCompositeEffect->SetInt( SpecularPowerHdl, pMaterial->nShininess ) );

		UINT techID = ((pMaterial->pDiffuseTex != 0) & 0x1) | (((pMaterial->pSpecularTex != 0) & 0x1)<<1);
		if (pMaterial->pDiffuseTex)
		{
			m_pCompositeEffect->SetTexture(compDiffuseTexHdl, pMaterial->pDiffuseTex);
		}
		if (pMaterial->pSpecularTex)
		{
			m_pCompositeEffect->SetTexture(compSpecularTexHdl, pMaterial->pSpecularTex);
		}
		m_pCompositeEffect->SetTechnique(compositeTech[techID]);
		RenderSubset( iSubset, m_pCompositeEffect);
	}

	//D3DXSaveTextureToFile(L"composite.jpg", D3DXIFF_JPG, m_pRenderTexture[TEX_COMPOSITE], NULL);

	return hr;
}
