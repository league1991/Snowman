#pragma once
#include <deque>

using namespace std;

#define GET_SET(var, Name) float Get##Name(){return var;} void Set##Name(float v){var = v;}

class HDRRenderer
{
public:
	HDRRenderer(void);
	~HDRRenderer(void);

	HRESULT		OnCreate(IDirect3DDevice9* pd3dDevice, int width, int height);
	HRESULT		OnReset(IDirect3DDevice9* pd3dDevice);
	HRESULT		OnLost();
	HRESULT		OnDestroy();

	GET_SET(m_gaussianMul, GaussMultiplier)
	GET_SET(m_gaussianVariance, GaussStdVariance)
	GET_SET(m_exposure, Exposure)
	GET_SET(m_brightPassThreshold, BrightThreshold)
	GET_SET(m_white, WhiteFactor)

	HRESULT		Render(	LPDIRECT3DTEXTURE9 oriTexture, 
						LPDIRECT3DSURFACE9 backBuffer,
						IDirect3DDevice9* device);

private:
	void		InitPtr();

	HRESULT		SetDownSample(int w, int h);
	HRESULT		SetBlurKernel(int w, int h);

	HRESULT		BrightPass(LPDIRECT3DTEXTURE9 oriTexture, IDirect3DDevice9* device);
	HRESULT		LuminancePass(LPDIRECT3DTEXTURE9 oriTexture, IDirect3DDevice9* device);
	HRESULT		BlurPass(LPDIRECT3DTEXTURE9 oriTexture,IDirect3DDevice9* device);
	HRESULT		FinalPass(LPDIRECT3DTEXTURE9 oriTexture,IDirect3DDevice9* device, LPDIRECT3DSURFACE9 backBuffer);

	inline float ComputeGaussianValue( float x, float std_deviation )
	{
		return ( 1.0f / (sqrt(2.0f * D3DX_PI) * std_deviation) )
			* expf( (-x*x) / (2.0f * std_deviation * std_deviation) );
	}
	void		ComputeKernel( int w, int h, float* hKernel, float* vKernel, float* hOffset, float* vOffset, int length);
	void		GetTextureDim(LPDIRECT3DTEXTURE9 tex, int&w, int& h);

	int							m_width, m_height;

	int							m_brightPassFactor;
	int							m_downSampleFactor;

	float						m_brightPassThreshold;
	float						m_exposure;
	float						m_white;

	// Gaussian
	float						m_gaussianMul, m_gaussianVariance;

	// textures
	LPDIRECT3DTEXTURE9			m_brightPassTex;
	deque<LPDIRECT3DTEXTURE9>	m_luminanceTexList;
	LPDIRECT3DTEXTURE9			m_downSampledTex,m_horiBlurTex,m_veriBlurTex;

	// surfaces
	LPDIRECT3DSURFACE9			m_brightPassSurf;
	deque<LPDIRECT3DSURFACE9>	m_luminanceSurfList;
	LPDIRECT3DSURFACE9			m_downSampledSurf, m_horiBlurSurf, m_veriBlurSurf;

	// effects
	ID3DXEffect*				m_processEffect;
};
