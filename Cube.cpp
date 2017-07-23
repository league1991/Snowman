// Cube.cpp: implementation of the CCube class.
//
//////////////////////////////////////////////////////////////////////
#include "DXUT.h"
#include "Cube.h"
#include "SDKmisc.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SkyBox::SkyBox(LPDIRECT3DDEVICE9 pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;
	m_pVertexBuffer = NULL;
	m_pCubeTexture = NULL;
	m_pSphereTexture = NULL;
	
	//设置立方体的初始值
	m_rWidth = 100.0;
	m_rHeight = 100.0;
	m_rDepth = 100.0;
	m_rX = 0.0;
	m_rY = 0.0;
	m_rZ = 0.0;

	m_skyType = SKY_CUBE;

	//设置材质的初始值 (R, G, B, A)
	D3DCOLORVALUE rgbaDiffuse  = {1.0, 1.0, 1.0, 0.0,};
	D3DCOLORVALUE rgbaAmbient  = {1.0, 1.0, 1.0, 0.0,};
	D3DCOLORVALUE rgbaSpecular = {0.0, 0.0, 0.0, 0.0,};
	D3DCOLORVALUE rgbaEmissive = {0.0, 0.0, 0.0, 0.0,};
	
	SetMaterial(rgbaDiffuse, rgbaAmbient, rgbaSpecular, rgbaEmissive, 0);
	
	//初始化顶点缓冲
    if(SUCCEEDED(CreateVertexBuffer()))
	{
		UpdateVertices();
	}

	// 初始化effect
	WCHAR str[MAX_PATH];
	HRESULT hr;
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;
	m_pEffect = NULL;

	// Read the D3DX effect file
	V( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"skybox.fx" ) );
	V( D3DXCreateEffectFromFile( pD3DDevice, L"skybox.fx", NULL, NULL, dwShaderFlags, NULL, &m_pEffect, NULL ) );
	m_worldViewMatHdl = m_pEffect->GetParameterByName(0, "matView");
	m_projMatHdl      = m_pEffect->GetParameterByName(0, "matProj");
	m_envTexHdl      = m_pEffect->GetParameterByName(0, "envTex");
}

SkyBox::~SkyBox()
{
	SafeRelease(m_pVertexBuffer);
	SafeRelease(m_pEffect);
	SafeRelease(m_pCubeTexture);
	SafeRelease(m_pSphereTexture);
}

bool SkyBox::Render()
{
	HRESULT hr;

	m_pD3DDevice->SetStreamSource(0, m_pVertexBuffer,0, sizeof(CUBE_CUSTOMVERTEX));
    m_pD3DDevice->SetFVF(CUBE_D3DFVF_CUSTOMVERTEX);

	m_pEffect->SetMatrix(m_worldViewMatHdl, &m_worldViewMat);
	m_pEffect->SetMatrix(m_projMatHdl, &m_projMat);

	D3DXHANDLE envFactorHdl = m_pEffect->GetParameterBySemantic(0,"EnvFactor");
	hr = m_pEffect->SetFloat(envFactorHdl, 4.0f);

	if (m_skyType == SKY_CUBE)
	{
		D3DXHANDLE tech = m_pEffect->GetTechniqueByName("CubeTech");

		m_pEffect->SetTechnique(tech);
		hr = m_pEffect->SetTexture(m_envTexHdl, m_pCubeTexture);	
	}
	else
	{
		D3DXHANDLE tech = m_pEffect->GetTechniqueByName("SphereTech");
		D3DXHANDLE uOffsetHdl = m_pEffect->GetParameterBySemantic(0,"SphereUVOffset");
		m_pEffect->SetTechnique(tech);
		hr = m_pEffect->SetTexture(m_envTexHdl, m_pSphereTexture);
		float offset[2] = {-0.25, 0.002};
		hr = m_pEffect->SetValue(uOffsetHdl, offset, sizeof(float)*2);	
	}

	UINT nPasses = 0;
	m_pEffect->Begin(&nPasses,  D3DXFX_DONOTSAVESTATE );
	for (UINT i = 0; i < nPasses; ++i)
	{
		m_pEffect->BeginPass(i);
		
		//设置材质
		if(FAILED(m_pD3DDevice->SetMaterial(&m_matMaterial)))
		{
			OutputDebugString(L"SetMaterial Failed!\n");
		}
		
		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 12);
		m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, false);
		m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, false);
		m_pEffect->EndPass();
	}
	m_pD3DDevice->SetRenderState(D3DRS_ZENABLE, true);
	m_pD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, true);
	m_pEffect->End();

	return true;
}

HRESULT SkyBox::CreateVertexBuffer()
{
    //创建顶点缓冲
    if(FAILED(m_pD3DDevice->CreateVertexBuffer(36 * sizeof(CUBE_CUSTOMVERTEX),
                                               0, CUBE_D3DFVF_CUSTOMVERTEX,
                                               D3DPOOL_DEFAULT, &m_pVertexBuffer, NULL)))
    {
        return E_FAIL;
    }

    return S_OK;
}

bool SkyBox::UpdateVertices()
{
	VOID* pVertices;
	D3DVECTOR vNormal;
	
	CUBE_CUSTOMVERTEX cvVertices[] =
	{	

		//顶面顶点
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 0
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 1
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 2
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 3
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 4
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 5
		
		//侧面
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 6
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 7
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 8
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 9
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 10
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 11
		
		
		//侧面
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 12
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 13
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 14
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 15
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 16
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 17
		
		
		//侧面
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 18
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 19
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 20
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 21
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 22
		{m_rX + (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 23
		
		
		//侧面
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 24
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 25
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 26
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 27
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 28
		{m_rX - (m_rWidth / 2), m_rY + (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 29
		
		
		//底面
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,},		//Vertex 30
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 31
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 32
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,},		//Vertex 33
		{m_rX - (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ - (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,},		//Vertex 34
		{m_rX + (m_rWidth / 2), m_rY - (m_rHeight / 2), m_rZ + (m_rDepth / 2), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,},		//Vertex 35
	};

	//设置所有顶点的法线向量
	int i;
	//wchar_t DEBUGMSG[255];
	
	for(i = 0; i < 36; i += 3)
	{
		vNormal = GetTriangeNormal(&D3DXVECTOR3(cvVertices[i].x, cvVertices[i].y, cvVertices[i].z), &D3DXVECTOR3(cvVertices[i + 1].x, cvVertices[i + 1].y, cvVertices[i + 1].z), &D3DXVECTOR3(cvVertices[i + 2].x, cvVertices[i + 2].y, cvVertices[i + 2].z));
		
		cvVertices[i].nx = vNormal.x;
		cvVertices[i].ny = vNormal.y;
		cvVertices[i].nz = vNormal.z;
		
		cvVertices[i + 1].nx = vNormal.x;
		cvVertices[i + 1].ny = vNormal.y;
		cvVertices[i + 1].nz = vNormal.z;
		
		cvVertices[i + 2].nx = vNormal.x;
		cvVertices[i + 2].ny = vNormal.y;
		cvVertices[i + 2].nz = vNormal.z;
		
		//wsprintf(DEBUGMSG, L"Vertices %d - %d: x = %f, y = %f, z = %f\n", i, i + 2, vNormal.x, vNormal.y, vNormal.z);
		//OutputDebugString(DEBUGMSG);
	}
	

	//锁定顶点缓冲
    if(FAILED(m_pVertexBuffer->Lock(0, sizeof(cvVertices), (void**)&pVertices, 0)))
    {
        return false;
    }

    //拷贝顶点的值
    memcpy(pVertices, cvVertices, sizeof(cvVertices));

    //解锁
    m_pVertexBuffer->Unlock();

	return true;
}

//设置立方体大小
bool SkyBox::SetSize(float rWidth, float rHeight, float rDepth)
{
	m_rWidth = rWidth;
	m_rHeight = rHeight;
	m_rDepth = rDepth;

	UpdateVertices();

	return true;
}
//设置立方体位置
bool SkyBox::SetPosition(float x, float y, float z)
{
	m_rX = x;
	m_rY = y;
	m_rZ = z;

	UpdateVertices();

	return true;
}

bool SkyBox::SetCubeTexture(LPCWSTR szTextureFilePath)
{
	SAFE_RELEASE(m_pCubeTexture);
	if(FAILED(D3DXCreateCubeTextureFromFile(m_pD3DDevice, szTextureFilePath, &m_pCubeTexture)))
	{
		return false;
	}
	
	m_skyType = SKY_CUBE;
	//UpdateVertices();
	
	return true;
}

bool SkyBox::SetMaterial(D3DCOLORVALUE rgbaDiffuse, D3DCOLORVALUE rgbaAmbient, D3DCOLORVALUE rgbaSpecular, D3DCOLORVALUE rgbaEmissive, float rPower)
{
	//设置漫反射
	m_matMaterial.Diffuse = rgbaDiffuse; 
	
	//设置环境光 
	m_matMaterial.Ambient = rgbaAmbient; 
	
	//设置镜面反射
	m_matMaterial.Specular = rgbaSpecular; 
	m_matMaterial.Power = rPower;
	
	//设置发射 
	m_matMaterial.Emissive = rgbaEmissive;
	
	return true;
}

D3DVECTOR SkyBox::GetTriangeNormal(D3DXVECTOR3 *vVertex1, D3DXVECTOR3 *vVertex2, D3DXVECTOR3 *vVertex3)
{
	D3DXVECTOR3 vNormal;
	D3DXVECTOR3 v1;
	D3DXVECTOR3 v2;
	
	D3DXVec3Subtract(&v1, vVertex2, vVertex1);
	D3DXVec3Subtract(&v2, vVertex3, vVertex1);
	
	D3DXVec3Cross(&vNormal, &v1, &v2);
	
	D3DXVec3Normalize(&vNormal, &vNormal);
	
	return vNormal;
}

bool SkyBox::SetMatrix( const D3DXMATRIXA16& worldViewMat, const D3DXMATRIXA16& projMat )
{
	m_worldViewMat = worldViewMat;
	m_projMat = projMat;
	return true;
}

HRESULT SkyBox::OnResetDevice( IDirect3DDevice9* pd3dDevice )
{
	HRESULT hr = S_OK;
	if (m_pEffect)
	{
		m_pEffect->OnResetDevice();
	}
	return hr;
}

HRESULT SkyBox::OnLostDevice()
{
	HRESULT hr = S_OK;
	if (m_pEffect)
	{
		m_pEffect->OnLostDevice();
	}
	return hr;
}

HRESULT SkyBox::OnDestroyDevice()
{
	HRESULT hr = S_OK;
	SAFE_RELEASE( m_pEffect );
	SAFE_RELEASE( m_pCubeTexture );
	SAFE_RELEASE(m_pVertexBuffer);
	return hr;
}

bool SkyBox::SetSphereTexture( LPCWSTR szTextureFilePath )
{
	SAFE_RELEASE(m_pSphereTexture);
	if(FAILED(D3DXCreateTextureFromFile(m_pD3DDevice, szTextureFilePath, &m_pSphereTexture)))
	{
		return false;
	}
	m_skyType = SKY_SPHERE;
	return true;
}
