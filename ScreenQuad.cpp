#include "DXUT.h"
#include "ScreenQuad.h"

ScreenQuad::ScreenQuad(void)
{
}

ScreenQuad::~ScreenQuad(void)
{
}

void ScreenQuad::DrawFullScreen( LPDIRECT3DDEVICE9 device, int imgWidth, int imgHeight )
{
	float x0 = -1.f;
	float y0 = -1.f;
	float width = 2.0f;
	float height = 2.0f;
	struct Vertex_V3T2
	{
		float m_Position[3]; // ¶¥µãÎ»ÖÃ
		float m_Texcoord[2]; // ÌùÍ¼×ø±ê
	};

	D3DXMATRIX ident_mat(
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1);

	device->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&ident_mat);
	device->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&ident_mat);
	device->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&ident_mat);

	float fTexelW = 1.0f/(float)imgWidth;
	float fTexelH = 1.0f/(float)imgHeight;

	const float z = 0.f;
	Vertex_V3T2 quad[4];

	quad[0].m_Position[0] = x0;
	quad[0].m_Position[1] = y0;
	quad[0].m_Position[2] = z;
	quad[0].m_Texcoord[0] = 0.0f + fTexelW*0.5f;
	quad[0].m_Texcoord[1] = 1.0f + fTexelH*0.5f;

	quad[1].m_Position[0] = x0 + width;
	quad[1].m_Position[1] = y0;
	quad[1].m_Position[2] = z;
	quad[1].m_Texcoord[0] = 1.0f + fTexelW*0.5f;
	quad[1].m_Texcoord[1] = 1.0f + fTexelH*0.5f;

	quad[2].m_Position[0] = x0;
	quad[2].m_Position[1] = y0 + height;
	quad[2].m_Position[2] = z;
	quad[2].m_Texcoord[0] = 0.0f + fTexelW*0.5f;
	quad[2].m_Texcoord[1] = 0.0f + fTexelH*0.5f;

	quad[3].m_Position[0] = x0 + width;
	quad[3].m_Position[1] = y0 + height;
	quad[3].m_Position[2] = z;
	quad[3].m_Texcoord[0] = 1.0f + fTexelW*0.5f;
	quad[3].m_Texcoord[1] = 0.0f + fTexelH*0.5f;

	device->SetFVF(D3DFVF_XYZ|D3DFVF_TEX1);
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, quad, sizeof(Vertex_V3T2));
}
