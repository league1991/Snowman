#pragma once

class ScreenQuad
{
public:
	ScreenQuad(void);
	~ScreenQuad(void);

	static void DrawFullScreen(LPDIRECT3DDEVICE9 device, int imgWidth, int imgHeight );

};
