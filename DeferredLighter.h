#pragma once

#include "DXUTcamera.h"

enum TextureType
{
	TEX_NORMAL,
	TEX_POS,
	TEX_LIGHT,
	TEX_COMPOSITE,
	TEX_TEXTURECOUNT
};

class DeferredLighter
{
public:
	DeferredLighter(CMeshLoader* meshloader, CFirstPersonCamera* camera, SkyBox* skybox);
	~DeferredLighter(void);
	void						SetTime(double time, float elapsedTime);
	HRESULT						OnCreate(IDirect3DDevice9* pd3dDevice, int width, int height);
	HRESULT						OnResetDevice(IDirect3DDevice9* pd3dDevice);
	HRESULT						OnRender(IDirect3DDevice9* pd3dDevice);
	HRESULT						OnLostDevice();
	HRESULT						OnDestroyDevice();
	
	LPDIRECT3DTEXTURE9			GetRenderTexture(TextureType tex);
private:
	void RenderSubset( UINT iSubset, ID3DXEffect* effect );

	HRESULT						GeometryPass(IDirect3DDevice9* pd3dDevice);
	HRESULT						LightingPass(IDirect3DDevice9* pd3dDevice);
	HRESULT						CompositePass(IDirect3DDevice9* pd3dDevice);

	SkyBox*						m_skyBox;
	CFirstPersonCamera*			m_Camera;				 // camera
	CMeshLoader*                m_MeshLoader;            // Loads a mesh from an .obj file

	D3DXMATRIXA16				mView;
	D3DXMATRIXA16 				mProj;
	D3DXMATRIXA16 				mWorldViewProjection;

	D3DXVECTOR3					lightClr;
	D3DXVECTOR3					lightDir;

	ID3DXEffect*                m_pGeoEffect;        // D3DX effect interface
	ID3DXEffect*				m_pLightEffect;
	ID3DXEffect*				m_pCompositeEffect;

	D3DXHANDLE                  m_hAmbient ;
	D3DXHANDLE                  m_hDiffuse ;
	D3DXHANDLE                  m_hSpecular ;
	D3DXHANDLE                  m_hOpacity ;
	D3DXHANDLE                  m_hSpecularPower ;
	D3DXHANDLE                  m_hLightColor ;
	D3DXHANDLE                  m_hLightPosition ;
	D3DXHANDLE                  m_hCameraPosition ;
	D3DXHANDLE                  m_hTexture ;
	D3DXHANDLE                  m_hTime ;
	D3DXHANDLE                  m_hView ;
	D3DXHANDLE                  m_hWorldViewProjection ;

	LPDIRECT3DTEXTURE9			m_pRenderTexture[TEX_TEXTURECOUNT];
	LPDIRECT3DSURFACE9			m_pRenderSurface[TEX_TEXTURECOUNT];
	LPDIRECT3DSURFACE9			m_pBackBuffer, pTempSurface;
	int							m_width, m_height;

	double fTime;
	float fElapsedTime;
};
