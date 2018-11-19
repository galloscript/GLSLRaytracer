
#include <stdio.h>
#include <Windows.h>

#include "Common.h"
#include "CGPUDevice.h"

const static PIXELFORMATDESCRIPTOR kPixelFormatDescriptor =
{
    sizeof(PIXELFORMATDESCRIPTOR),          //    WORD  nSize;
    1,                                      //    WORD  nVersion;
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,  //    DWORD dwFlags;
    PFD_TYPE_RGBA,                          //    BYTE  iPixelType;
    16,                                     //    BYTE  cColorBits;
    0,                                      //    BYTE  cRedBits;
    0,                                      //    BYTE  cRedShift;
    0,                                      //    BYTE  cGreenBits;
    0,                                      //    BYTE  cGreenShift;
    0,                                      //    BYTE  cBlueBits;
    0,                                      //    BYTE  cBlueShift;
    0,                                      //    BYTE  cAlphaBits;
    0,                                      //    BYTE  cAlphaShift;
    0,                                      //    BYTE  cAccumBits;
    0,                                      //    BYTE  cAccumRedBits;
    0,                                      //    BYTE  cAccumGreenBits;
    0,                                      //    BYTE  cAccumBlueBits;
    0,                                      //    BYTE  cAccumAlphaBits;
    24,                                      //    BYTE  cDepthBits;
    0,                                      //    BYTE  cStencilBits;
    0,                                      //    BYTE  cAuxBuffers;
    0,                                      //    BYTE  iLayerType;
    0,                                      //    BYTE  bReserved;
    0,                                      //    DWORD dwLayerMask;
    0,                                      //    DWORD dwVisibleMask;
    0                                       //    DWORD dwDamageMask;
};

//static RECT kWindowRec = { 0, 0, 1280, 720 };
//static RECT kWindowRec = { 0, 0, 800, 400 };

static RECT kWindowRec = { 0, 0, 600, 600 };



//--------------------------------------------------------------------------//
static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    // salvapantallas
    if( uMsg==WM_SYSCOMMAND && (wParam==SC_SCREENSAVE || wParam==SC_MONITORPOWER) )
        return( 0 );

    // boton x o pulsacion de escape
    if( uMsg==WM_CLOSE || uMsg==WM_DESTROY || (uMsg==WM_KEYDOWN && wParam==VK_ESCAPE) )
    {
        PostQuitMessage(0);
        return( 0 );
    }

    if( uMsg==WM_CHAR )
    {
        if( wParam==VK_ESCAPE )
        {
            PostQuitMessage(0);
            return( 0 );
        }
    }

    return( DefWindowProc(hWnd,uMsg,wParam,lParam) );
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{ 
    WNDCLASS        wc;

    // create window
    ZeroMemory( &wc, sizeof(WNDCLASS) );
    wc.style         = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hPrevInstance;
    wc.lpszClassName = "smallSequel";

    if( !RegisterClass(&wc) )
        return( 0 );

    const int dws = WS_VISIBLE | WS_CAPTION | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU;
    AdjustWindowRect( &kWindowRec, dws, 0 );
    HWND hWnd = CreateWindowEx( WS_EX_APPWINDOW | WS_EX_WINDOWEDGE , wc.lpszClassName, "GPURaymarcher", dws,
                               (GetSystemMetrics(SM_CXSCREEN)-kWindowRec.right+kWindowRec.left)>>1,
                               (GetSystemMetrics(SM_CYSCREEN)-kWindowRec.bottom+kWindowRec.top)>>1,
                               kWindowRec.right-kWindowRec.left, kWindowRec.bottom-kWindowRec.top, 0, 0, hPrevInstance, 0 );

    if( !hWnd )
        return( 0 );

    HDC hDC=GetDC(hWnd);
    if( !hDC )
        return( 0 );

    // init opengl
    int pf = ChoosePixelFormat(hDC, &kPixelFormatDescriptor);
    if( !pf )
        return( 0 );
    
    if( !SetPixelFormat(hDC,pf,&kPixelFormatDescriptor) )
        return( 0 );

    HGLRC hRC = wglCreateContext(hDC);
    if( !hRC )
        return( 0 );

    if( !wglMakeCurrent(hDC,hRC) )
        return( 0 );

    CGPUDevice* lGPU = CGPUDevice::GetInstance();

    if( !lGPU->Init(kWindowRec.right, kWindowRec.bottom, hDC) )
    {
        exit(-1);
    }

    
    ShowCursor(1); 

    bool lDone = false;
    const float lBaseTime = GetTickCount() / 1000.f;
    float lAvgFPS = 0;

    bool lMousePressed = false;

    POINT lCursorPos  = {0,0};
    POINT lPrevCursorPos  = {0,0};
    POINT lCursorDisp = {0,0};
    
    

    while( !lDone )
    {
        float lFrameStartTime = (GetTickCount() / 1000.f) - lBaseTime;
        
        GetCursorPos(&lCursorPos);
        lCursorDisp.x = lCursorPos.x - lPrevCursorPos.x;
        lCursorDisp.y = lCursorPos.y - lPrevCursorPos.y;

        LARGE_INTEGER lFreq, lStart;
        LARGE_INTEGER lEnd;
        QueryPerformanceFrequency(&lFreq);
        QueryPerformanceCounter(&lStart);



        /* do something ... */

        MSG msg;
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) lDone = true;
            if (msg.message == WM_LBUTTONDOWN)
            {
                lMousePressed = true;
            }
            if (msg.message == WM_LBUTTONUP)
            {
                lMousePressed = false;
            }
            if (msg.message == WM_CHAR)
            {
                //msg.message == WM_LBUTTONUP
                if (msg.wParam >= 49 && msg.wParam <= 57)
                {
           
                }
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(lMousePressed)
        {
            lGPU->ResetFrame();
            
            lGPU->AddCameraXYZ((float)lCursorDisp.x, (float)lCursorDisp.y, 0);
        }

        lGPU->ExecuteComputeStep(lFrameStartTime);
        
        lGPU->Render(lFrameStartTime);

        SwapBuffers( hDC );

        lPrevCursorPos = lCursorPos;

        QueryPerformanceCounter(&lEnd);
        long double lFrameTime = ((long double)lEnd.QuadPart - (long double)lStart.QuadPart) / (long double)lFreq.QuadPart;


        static char lBuffer[100];
        //long double lFPS = 1.0f / ((lFrameTime > 0.f) ? lFrameTime : 0.001);
        long double lFPS = 1.0f / lFrameTime;
        lAvgFPS = float(lAvgFPS * 0.75f + lFPS * 0.25f);
        sprintf(lBuffer, "GPU Raytracer ~%.0Lf FPS - %d Samples", lAvgFPS, lGPU->GetSamplesCount());
        SetWindowText(hWnd, lBuffer);
    } 

    ExitProcess( 0 );

    return 0;
}
