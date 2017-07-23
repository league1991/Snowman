// Cube.h: interface for the CCube class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CUBE_H__AE25AF5F_AE56_48F5_99DC_47CAAA14F245__INCLUDED_)
#define AFX_CUBE_H__AE25AF5F_AE56_48F5_99DC_47CAAA14F245__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <d3dx9.h>

#if !defined(AFX_HELPER_H)
#define AFX_HELPER_H

#define SafeRelease(pInterface) if(pInterface != NULL) {pInterface->Release(); pInterface=NULL;}
#define SafeDelete(pObject) if(pObject != NULL) {delete pObject; pObject=NULL;}

#endif


//定义顶点格式
#define CUBE_D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

enum SkyType
{
	SKY_CUBE,
	SKY_SPHERE
};

class SkyBox  
{
public:
	SkyBox( LPDIRECT3DDEVICE9 pD3DDevice);
	virtual ~SkyBox();

	bool SetMatrix(const D3DXMATRIXA16& worldViewMat, const D3DXMATRIXA16& projMat);
	bool SetMaterial(D3DCOLORVALUE rgbaDiffuse, D3DCOLORVALUE rgbaAmbient, D3DCOLORVALUE rgbaSpecular, D3DCOLORVALUE rgbaEmissive, float rPower);
	bool SetCubeTexture(LPCWSTR szTextureFilePath);
	bool SetSphereTexture(LPCWSTR szTextureFilePath);
	bool SetSize(float rWidth, float rHeight, float rDepth);

	HRESULT						OnResetDevice(IDirect3DDevice9* pd3dDevice);
	HRESULT						OnLostDevice();
	HRESULT						OnDestroyDevice();
	bool Render();
private:
	D3DVECTOR GetTriangeNormal(D3DXVECTOR3* vVertex1, D3DXVECTOR3* vVertex2, D3DXVECTOR3* vVertex3);
	bool UpdateVertices();
	HRESULT CreateVertexBuffer( );
	bool SetPosition(float x, float y, float z);

	// D3D数据
	LPDIRECT3DDEVICE9			m_pD3DDevice;
	LPDIRECT3DVERTEXBUFFER9		m_pVertexBuffer;
	LPDIRECT3DCUBETEXTURE9		m_pCubeTexture;
	LPDIRECT3DTEXTURE9          m_pSphereTexture;
	D3DMATERIAL9				m_matMaterial;

	ID3DXEffect*                m_pEffect; 
	D3DXHANDLE					m_worldViewMatHdl, m_projMatHdl, m_envTexHdl;

	// 参数
	D3DXMATRIXA16				m_worldViewMat, m_projMat;
	
	SkyType						m_skyType;

	float m_rWidth;
	float m_rHeight;
	float m_rDepth;
	float m_rX;
	float m_rY;
	float m_rZ;
	
	//顶点结构
	struct CUBE_CUSTOMVERTEX
	{
		FLOAT x, y, z;		//三维坐标	
		FLOAT nx, ny, nz;	//光照法线
		FLOAT tu, tv;		//纹理坐标

		
	};
};

#endif // !defined(AFX_CUBE_H__AE25AF5F_AE56_48F5_99DC_47CAAA14F245__INCLUDED_)

