//--------------------------------------------------------------------------------------
// File: MeshFromOBJ.cpp
//
// This sample shows how an ID3DXMesh object can be created from mesh data stored in an 
// .obj file. It's convenient to use .x files when working with ID3DXMesh objects since 
// D3DX can create and fill an ID3DXMesh object directly from an .x file; however, it’s 
// also easy to initialize an ID3DXMesh object with data gathered from any file format 
// or memory resource.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#include "DXUT.h"
#include "DXUTcamera.h"
#include "DXUTsettingsdlg.h"
#include "SDKmisc.h"
#include "resource.h"
#include "meshloader.h"
#include "Cube.h"
#pragma warning(disable: 4995)
#pragma warning(default: 4995)

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 



//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------

bool                        g_bShowHelp = false;      // If true, it renders the UI control text
ID3DXFont*                  g_pFont = NULL;          // Font for drawing text
ID3DXSprite*                g_pTextSprite = NULL;    // Sprite for batching draw text calls
CDXUTDialogResourceManager  g_DialogResourceManager; // manager for shared resources of dialogs
CD3DSettingsDlg             g_SettingsDlg;          // Device settings dialog
CDXUTDialog                 g_HUD;                   // dialog for standard controls
CDXUTDialog                 g_SampleUI;              // dialog for sample specific controls

CMeshLoader                 g_MeshLoader;            // Loads a mesh from an .obj file
CFirstPersonCamera			g_Camera;				 // camera
SkyBox*						g_skyBox = NULL;
DeferredLighter*			g_deferredLighter = NULL;
HDRRenderer*				g_hdrRender = NULL;		 // hdr renderer

WCHAR g_strFileSaveMessage[MAX_PATH] = {0}; // Text indicating file write success/failure


int							g_width = 0, g_height = 0;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_STATIC              -1
enum UIElement
{
	IDC_TOGGLEFULLSCREEN = 1,
	IDC_TOGGLEREF,
	IDC_CHANGEDEVICE,
	IDC_SAVETOX,

	IDC_GAUSSIAN_MUL,
	IDC_GAUSSIAN_MUL_TXT,
	IDC_GAUSSIAN_VAR,
	IDC_GAUSSIAN_VAR_TXT,
	IDC_EXPOSURE,
	IDC_EXPOSURE_TXT,
	IDC_WHITE,
	IDC_WHITE_TXT,
	IDC_BRIGHT_THRESHOLD,
	IDC_BRIGHT_THRESHOLD_TXT
};


//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed,
                                  void* pUserContext );
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext );
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext );
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                void* pUserContext );
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext );
void CALLBACK OnRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext );
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext );
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext );
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext );
void CALLBACK OnLostDevice( void* pUserContext );
void CALLBACK OnDestroyDevice( void* pUserContext );

void InitApp();
void RenderText();
void RenderSubset( UINT iSubset, ID3DXEffect* effect );
void SaveMeshToXFile();

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR, int )
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    // Set the callback functions. These functions allow DXUT to notify
    // the application about device changes, user input, and windows messages.  The 
    // callbacks are optional so you need only set callbacks for events you're interested 
    // in. However, if you don't handle the device reset/lost callbacks then the sample 
    // framework won't be able to reset your device since the application must first 
    // release all device resources before resetting.  Likewise, if you don't handle the 
    // device created/destroyed callbacks then DXUT won't be able to 
    // recreate your device resources.
    DXUTSetCallbackD3D9DeviceAcceptable( IsDeviceAcceptable );
    DXUTSetCallbackD3D9DeviceCreated( OnCreateDevice );
    DXUTSetCallbackD3D9DeviceReset( OnResetDevice );
    DXUTSetCallbackD3D9FrameRender( OnRender );
    DXUTSetCallbackD3D9DeviceLost( OnLostDevice );
    DXUTSetCallbackD3D9DeviceDestroyed( OnDestroyDevice );
    DXUTSetCallbackMsgProc( MsgProc );
    DXUTSetCallbackKeyboard( KeyboardProc );
    DXUTSetCallbackFrameMove( OnFrameMove );
    DXUTSetCallbackDeviceChanging( ModifyDeviceSettings );

    // Show the cursor and clip it when in full screen
    DXUTSetCursorSettings( true, true );

    InitApp();

    // Initialize DXUT and create the desired Win32 window and Direct3D 
    // device for the application. Calling each of these functions is optional, but they
    // allow you to set several options which control the behavior of the framework.
    DXUTInit( true, true ); // Parse the command line and show msgboxes
    DXUTSetHotkeyHandling( true, true, true );  // handle the defaul hotkeys
    DXUTCreateWindow( L"Snowman" );
    DXUTCreateDevice( true, 800, 600 );

    // Pass control to DXUT for handling the message pump and 
    // dispatching render calls. DXUT will call your FrameMove 
    // and FrameRender callback when there is idle time between handling window messages.
    DXUTMainLoop();

    // Perform any application-level cleanup here. Direct3D device resources are released within the
    // appropriate callback functions and therefore don't require any cleanup code here.

    return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
    // Initialize dialogs
    g_SettingsDlg.Init( &g_DialogResourceManager );
	
    g_HUD.Init( &g_DialogResourceManager );
    g_SampleUI.Init( &g_DialogResourceManager );
    g_HUD.SetCallback( OnGUIEvent ); int iY = 24;
    g_HUD.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", 35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_TOGGLEREF,        L"Toggle REF (F3)",    35, iY += 24, 125, 22 );
    g_HUD.AddButton( IDC_CHANGEDEVICE,     L"Change device (F2)", 35, iY += 24, 125, 22, VK_F2 );
	

	g_HUD.AddStatic( IDC_GAUSSIAN_MUL_TXT, L"Gaussian Multiplier",35, iY += 48, 125, 22);
	g_HUD.AddSlider( IDC_GAUSSIAN_MUL,                            35, iY += 24, 125, 22, 0, 80, 12);
	g_HUD.AddStatic( IDC_GAUSSIAN_VAR_TXT, L"Gaussian Std. Variance",35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_GAUSSIAN_VAR,                            35, iY += 24, 125, 22, 1, 100, 50);
	g_HUD.AddStatic( IDC_EXPOSURE_TXT,     L"Exposure Factor",    35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_EXPOSURE,                                35, iY += 24, 125, 22, 0, 200, 120);
	g_HUD.AddStatic( IDC_WHITE_TXT,     L"White Factor",          35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_WHITE,                                   35, iY += 24, 125, 22, 0, 600, 115);
	g_HUD.AddStatic( IDC_BRIGHT_THRESHOLD_TXT,L"Bright Threshold",35, iY += 24, 125, 22);
	g_HUD.AddSlider( IDC_BRIGHT_THRESHOLD,                        35, iY += 24, 125, 22, 0, 200, 94);


    // Title font for comboboxes
	//g_SampleUI.SetCallback( OnGUIEvent ); iY = 10;
    g_SampleUI.SetFont( 1, L"Arial", 14, FW_BOLD );
    CDXUTElement* pElement = g_SampleUI.GetDefaultElement( DXUT_CONTROL_STATIC, 0 );
    if( pElement )
    {
        pElement->iFont = 1;
        pElement->dwTextFormat = DT_LEFT | DT_BOTTOM;
    }

    //g_SampleUI.AddButton( IDC_SAVETOX, L"Save Mesh To X file", 20, 50, 140, 24, 'X' );
}





//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable( D3DCAPS9* pCaps, D3DFORMAT AdapterFormat,
                                  D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext )
{
    // No fallback defined by this app, so reject any device that 
    // doesn't support at least ps2.0
    if( pCaps->PixelShaderVersion < D3DPS_VERSION( 2, 0 ) )
        return false;

    // Skip backbuffer formats that don't support alpha blending
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    if( FAILED( pD3D->CheckDeviceFormat( pCaps->AdapterOrdinal, pCaps->DeviceType,
                                         AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
                                         D3DRTYPE_TEXTURE, BackBufferFormat ) ) )
        return false;

    return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
    assert( DXUT_D3D9_DEVICE == pDeviceSettings->ver );

    HRESULT hr;
    IDirect3D9* pD3D = DXUTGetD3D9Object();
    D3DCAPS9 caps;

    V( pD3D->GetDeviceCaps( pDeviceSettings->d3d9.AdapterOrdinal,
                            pDeviceSettings->d3d9.DeviceType,
                            &caps ) );

    // If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
    // then switch to SWVP.
    if( ( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT ) == 0 ||
        caps.VertexShaderVersion < D3DVS_VERSION( 1, 1 ) )
    {
        pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    // Debugging vertex shaders requires either REF or software vertex processing 
    // and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
    if( pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF )
    {
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
        pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
        pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }
#endif
#ifdef DEBUG_PS
    pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif

    // Enable anti-aliasing for HAL devices which support it
//    CD3D9Enumeration* pEnum = DXUTGetD3D9Enumeration();
//    CD3D9EnumDeviceSettingsCombo* pCombo = pEnum->GetDeviceSettingsCombo( &pDeviceSettings->d3d9 );

//     if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_HAL &&
//         pCombo->multiSampleTypeList.Contains( D3DMULTISAMPLE_4_SAMPLES ) && 0)
//     {
//         pDeviceSettings->d3d9.pp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
//         pDeviceSettings->d3d9.pp.MultiSampleQuality = 0;
//     }
    // For the first device created if its a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if( s_bFirstTime )
    {
        s_bFirstTime = false;
        if( pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF )
            DXUTDisplaySwitchingToREFWarning( pDeviceSettings->ver );
    }

    return true;
}

float GetSliderValue(UIElement sliderID)
{
	return g_HUD.GetSlider(sliderID)->GetValue() / 100.f;
}

void  SetSliderValue(UIElement sliderID, float value)
{
	return g_HUD.GetSlider(sliderID)->SetValue(int(value * 100));
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc,
                                 void* pUserContext )
{
    HRESULT hr;

	// set screen dimension
	g_width = pBackBufferSurfaceDesc->Width;
	g_height = pBackBufferSurfaceDesc->Height;

    V_RETURN( g_DialogResourceManager.OnD3D9CreateDevice( pd3dDevice ) );
    V_RETURN( g_SettingsDlg.OnD3D9CreateDevice( pd3dDevice ) );

    // Initialize the font
    V_RETURN( D3DXCreateFont( pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
                              OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                              L"Arial", &g_pFont ) );

    // Create the mesh and load it with data already gathered from a file
    V_RETURN( g_MeshLoader.Create( pd3dDevice, L"snowman.obj" ) );

	// init skybox
	g_skyBox = new SkyBox(pd3dDevice);
	//if(!g_skyBox->SetCubeTexture(L"media\\skybox02.dds"))
	if(!g_skyBox->SetSphereTexture(L"FN_HDRI_027.hdr"))
		return S_FALSE;

	g_deferredLighter = new DeferredLighter(&g_MeshLoader, &g_Camera, g_skyBox);
	V_RETURN( g_deferredLighter->OnCreate(pd3dDevice, g_width, g_height));

	g_hdrRender = new HDRRenderer();
	V_RETURN( g_hdrRender->OnCreate(pd3dDevice, g_width, g_height));

	g_hdrRender->SetGaussMultiplier(GetSliderValue(IDC_GAUSSIAN_MUL));
	g_hdrRender->SetGaussStdVariance(GetSliderValue(IDC_GAUSSIAN_VAR));
	g_hdrRender->SetExposure(GetSliderValue(IDC_EXPOSURE));
	g_hdrRender->SetBrightThreshold(GetSliderValue(IDC_BRIGHT_THRESHOLD));
	g_hdrRender->SetWhiteFactor(GetSliderValue(IDC_WHITE));
    return hr;
}





//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice( IDirect3DDevice9* pd3dDevice,
                                const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    HRESULT hr;

    V_RETURN( g_DialogResourceManager.OnD3D9ResetDevice() );
    V_RETURN( g_SettingsDlg.OnD3D9ResetDevice() );

    if( g_pFont )
        V_RETURN( g_pFont->OnResetDevice() );

    // Create a sprite to help batch calls when drawing many lines of text
    V_RETURN( D3DXCreateSprite( pd3dDevice, &g_pTextSprite ) );

    // Setup the camera
    float fAspectRatio = pBackBufferSurfaceDesc->Width / ( FLOAT )pBackBufferSurfaceDesc->Height;
	D3DXVECTOR3 vecEye( -45.6, 16.0f, -115.1f );
	D3DXVECTOR3 vecAt ( -45.1f, 16.1f, -114.3f );
	g_Camera.SetProjParams( D3DX_PI / 4, fAspectRatio, 10.f, 5000.0f );
	g_Camera.SetViewParams( &vecEye, &vecAt );
	g_Camera.SetScalers( 0.03f, 30.0f );
	g_Camera.SetEnableYAxisMovement(false);
	g_Camera.SetLeftRightRotate(true);

	V_RETURN( g_deferredLighter->OnResetDevice(pd3dDevice));
	V_RETURN( g_hdrRender->OnReset(pd3dDevice));

    g_HUD.SetLocation( pBackBufferSurfaceDesc->Width - 170, 0 );
    g_HUD.SetSize( 170, 170 );
    g_HUD.Refresh();

	g_SampleUI.SetLocation( pBackBufferSurfaceDesc->Width - 170, pBackBufferSurfaceDesc->Height - 350 );
	g_SampleUI.SetSize( 170, 300 );
	g_SampleUI.Refresh();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove( double fTime, float fElapsedTime, void* pUserContext )
{
    // Update the camera's position based on user input 
    g_Camera.FrameMove( fElapsedTime );
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnRender( IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext )
{
    // If the settings dialog is being shown, then
    // render it instead of rendering the app's scene
    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.OnRender( fElapsedTime );
        return;
    }

    HRESULT hr;

    // Clear the render target and the zbuffer 
 	IDirect3DSurface9* g_pBackBuffer = NULL;
 	pd3dDevice->GetRenderTarget(0, &g_pBackBuffer);

    // Render the scene
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
	{
		// ----------------------------------------------------
		// Deferred Lighting Pass
		// ----------------------------------------------------	
		g_deferredLighter->OnRender(pd3dDevice);


		// ----------------------------------------------------
		// HDR Pass
		// ----------------------------------------------------	
		g_hdrRender->Render(g_deferredLighter->GetRenderTexture(TEX_COMPOSITE), g_pBackBuffer, pd3dDevice);

		pd3dDevice->SetRenderTarget(0, g_pBackBuffer);
		pd3dDevice->SetRenderTarget(1, NULL);

        RenderText();
        V( g_HUD.OnRender( fElapsedTime ) );
        V( g_SampleUI.OnRender( fElapsedTime ) );
        V( pd3dDevice->EndScene() );
	}
	
	g_pBackBuffer->Release();

// 	D3DXSaveTextureToFile(L"diffuse.jpg", D3DXIFF_JPG, g_pRenderTexture[TEX_DIFFUSE], NULL);
// 	D3DXSaveTextureToFile(L"pos.jpg", D3DXIFF_JPG, g_pRenderTexture[TEX_POS], NULL);
// 	D3DXSaveTextureToFile(L"normal.jpg", D3DXIFF_JPG, g_pRenderTexture[TEX_NORMAL], NULL);
//	D3DXSaveTextureToFile(L"light.png", D3DXIFF_PNG, g_pRenderTexture[TEX_LIGHT], NULL);
}



//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------

void RenderText()
{
    // The helper object simply helps keep track of text position, and color
    // and then it calls pFont->DrawText( m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr );
    // If NULL is passed in as the sprite object, then it will work however the 
    // pFont->DrawText() will not be batched together.  Batching calls will improves performance.
    CDXUTTextHelper txtHelper( g_pFont, g_pTextSprite, 15 );

    // Output statistics
    txtHelper.Begin();
    txtHelper.SetInsertionPos( 5, 5 );
    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 0.0f, 1.0f ) );
    txtHelper.DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
    txtHelper.DrawTextLine( DXUTGetDeviceStats() );

    txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
    txtHelper.DrawTextLine( g_strFileSaveMessage );

    // Draw help
    if( g_bShowHelp )
    {
        const D3DSURFACE_DESC* pd3dsdBackBuffer = DXUTGetD3D9BackBufferSurfaceDesc();
        txtHelper.SetInsertionPos( 10, pd3dsdBackBuffer->Height - 15 * 5 );
        txtHelper.SetForegroundColor( D3DCOLOR_ARGB( 200, 50, 50, 50 ) );
        txtHelper.DrawTextLine( L"Controls (F1 to hide):" );

        txtHelper.SetInsertionPos( 20, pd3dsdBackBuffer->Height - 15 * 4 );
        txtHelper.DrawTextLine( L"Rotate model: Left mouse button\n"
                                L"Rotate camera: Right mouse button\n"
                                L"Zoom camera: Mouse wheel scroll\n" );

        txtHelper.SetInsertionPos( 250, pd3dsdBackBuffer->Height - 15 * 4 );
        txtHelper.DrawTextLine( L"Hide help: F1\n" );
        txtHelper.DrawTextLine( L"Quit: ESC\n" );
    }
    else
    {
        txtHelper.SetForegroundColor( D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f ) );
        txtHelper.DrawTextLine( L"Press F1 for help" );
    }

    txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
                          void* pUserContext )
{
    // Always allow dialog resource manager calls to handle global messages
    // so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    if( g_SettingsDlg.IsActive() )
    {
        g_SettingsDlg.MsgProc( hWnd, uMsg, wParam, lParam );
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;
	
    *pbNoFurtherProcessing = g_SampleUI.MsgProc( hWnd, uMsg, wParam, lParam );
    if( *pbNoFurtherProcessing )
        return 0;

    // Pass all remaining windows messages to camera so it can respond to user input
    g_Camera.HandleMessages( hWnd, uMsg, wParam, lParam );

    return 0;
}


//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
    if( bKeyDown )
    {
        switch( nChar )
        {
            case VK_F1:
                g_bShowHelp = !g_bShowHelp; break;
        }
    }
}


//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
	WCHAR str[100];
	float v = 0;
    switch( nControlID )
    {
        case IDC_TOGGLEFULLSCREEN:
            DXUTToggleFullScreen(); break;
        case IDC_TOGGLEREF:
            DXUTToggleREF(); break;
        case IDC_CHANGEDEVICE:
            g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() ); break;
        case IDC_SAVETOX:
			SaveMeshToXFile(); break;
		case IDC_GAUSSIAN_MUL:
			v = GetSliderValue(IDC_GAUSSIAN_MUL);
			g_hdrRender->SetGaussMultiplier(v);
			swprintf_s( str, 100, L"Gaussian Multiplier: (%.2f)", v );
			g_HUD.GetStatic( IDC_GAUSSIAN_MUL_TXT )->SetText( str );
			break;
		case IDC_GAUSSIAN_VAR:
			v = GetSliderValue(IDC_GAUSSIAN_VAR);
			g_hdrRender->SetGaussStdVariance(v);
			swprintf_s( str, 100, L"Gaussian Std. Variance: (%.2f)", v );
			g_HUD.GetStatic( IDC_GAUSSIAN_VAR_TXT )->SetText( str );
			break;
		case IDC_EXPOSURE:
			v = GetSliderValue(IDC_EXPOSURE);
			g_hdrRender->SetExposure(v);
			swprintf_s( str, 100, L"Exposure: (%.2f)", v );
			g_HUD.GetStatic( IDC_EXPOSURE_TXT )->SetText( str );
			break;
		case IDC_BRIGHT_THRESHOLD:
			v = GetSliderValue(IDC_BRIGHT_THRESHOLD);
			g_hdrRender->SetBrightThreshold(v);
			swprintf_s( str, 100, L"Bright Threshold: (%.2f)", v );
			g_HUD.GetStatic( IDC_BRIGHT_THRESHOLD_TXT )->SetText( str );
			break;
		case IDC_WHITE:
			v = GetSliderValue(IDC_WHITE);
			g_hdrRender->SetWhiteFactor(v);
			swprintf_s( str, 100, L"White Factor: (%.2f)", v );
			g_HUD.GetStatic( IDC_WHITE_TXT )->SetText( str );
			break;
    }
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9LostDevice();
    g_SettingsDlg.OnD3D9LostDevice();
    if( g_pFont )
        g_pFont->OnLostDevice();

    SAFE_RELEASE( g_pTextSprite );

	g_skyBox->OnLostDevice();
	g_deferredLighter->OnLostDevice();
	g_hdrRender->OnLost();
}


//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice( void* pUserContext )
{
    g_DialogResourceManager.OnD3D9DestroyDevice();
    g_SettingsDlg.OnD3D9DestroyDevice();
	SAFE_RELEASE( g_pFont );

    g_MeshLoader.Destroy();

 	g_skyBox->OnDestroyDevice();
	SAFE_DELETE(g_skyBox);

	
	if (g_hdrRender)
	{
		g_hdrRender->OnDestroy();
	}
	SAFE_DELETE(g_hdrRender);

	if (g_deferredLighter)
	{
		g_deferredLighter->OnDestroyDevice();
	}
	SAFE_DELETE(g_deferredLighter);
}

//--------------------------------------------------------------------------------------
// Saves the mesh to X-file
//--------------------------------------------------------------------------------------
void SaveMeshToXFile()
{
    HRESULT hr;

    // Fill out D3DXMATERIAL structures
    UINT numMaterials = g_MeshLoader.GetNumMaterials();
    D3DXMATERIAL* pMaterials = new D3DXMATERIAL[numMaterials];
    char* pStrTexture = new char[MAX_PATH * numMaterials];
    if( ( pMaterials != NULL ) && ( pStrTexture != NULL ) )
    {
        for( UINT i = 0; i < g_MeshLoader.GetNumMaterials(); i++ )
        {
            Material* pMat = g_MeshLoader.GetMaterial( i );
            if( pMat != NULL )
            {
                pMaterials[i].MatD3D.Ambient.r = pMat->vAmbient.x;
                pMaterials[i].MatD3D.Ambient.g = pMat->vAmbient.y;
                pMaterials[i].MatD3D.Ambient.b = pMat->vAmbient.z;
                pMaterials[i].MatD3D.Ambient.a = pMat->fAlpha;

                pMaterials[i].MatD3D.Diffuse.r = pMat->vDiffuse.x;
                pMaterials[i].MatD3D.Diffuse.g = pMat->vDiffuse.y;
                pMaterials[i].MatD3D.Diffuse.b = pMat->vDiffuse.z;
                pMaterials[i].MatD3D.Diffuse.a = pMat->fAlpha;

                pMaterials[i].MatD3D.Specular.r = pMat->vSpecular.x;
                pMaterials[i].MatD3D.Specular.g = pMat->vSpecular.y;
                pMaterials[i].MatD3D.Specular.b = pMat->vSpecular.z;
                pMaterials[i].MatD3D.Specular.a = pMat->fAlpha;

                pMaterials[i].MatD3D.Emissive.r = 0;
                pMaterials[i].MatD3D.Emissive.g = 0;
                pMaterials[i].MatD3D.Emissive.b = 0;
                pMaterials[i].MatD3D.Emissive.a = 0;

                pMaterials[i].MatD3D.Power = ( float )pMat->nShininess;

                WideCharToMultiByte( CP_ACP, 0, pMat->strDiffuseTex, -1, ( pStrTexture + i * MAX_PATH ), MAX_PATH, NULL,
                                     NULL );
                pMaterials[i].pTextureFilename = ( pStrTexture + i * MAX_PATH );
            }

        }

        // Write to file in same directory where the .obj file was found
        WCHAR strBuf[ MAX_PATH ];
        swprintf_s( strBuf, MAX_PATH - 1, L"%s\\%s", g_MeshLoader.GetMediaDirectory(), L"MeshFromOBJ.x" );
        hr = D3DXSaveMeshToX( strBuf, g_MeshLoader.GetMesh(), NULL,
                              pMaterials, NULL, numMaterials,
                              D3DXF_FILEFORMAT_TEXT );

        if( SUCCEEDED( hr ) )
        {
            swprintf_s( g_strFileSaveMessage, MAX_PATH - 1, L"Created %s", strBuf );
        }
        else
        {
            DXTRACE_ERR( L"SaveMeshToXFile()::D3DXSaveMeshToX", hr );
            swprintf_s( g_strFileSaveMessage, MAX_PATH - 1, L"Error creating %s, check debug output", strBuf );
        }
    }

    SAFE_DELETE_ARRAY( pMaterials );
    SAFE_DELETE_ARRAY( pStrTexture );
}



