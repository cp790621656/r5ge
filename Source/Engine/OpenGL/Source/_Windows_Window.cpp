#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

//=============================================================================================================
#if defined (_WINDOWS)
//=============================================================================================================

#include <windows.h>
using namespace R5;

//=============================================================================================================
// Defines pulled from <wglext.h>
//=============================================================================================================

#define WGL_DRAW_TO_WINDOW		0x2001
#define WGL_ACCELERATION		0x2003
#define WGL_SUPPORT_OPENGL		0x2010
#define WGL_DOUBLE_BUFFER		0x2011
#define WGL_COLOR_BITS			0x2014
#define WGL_RED_BITS			0x2015
#define WGL_GREEN_BITS			0x2017
#define WGL_BLUE_BITS			0x2019
#define WGL_ALPHA_BITS			0x201B
#define WGL_DEPTH_BITS			0x2022
#define WGL_STENCIL_BITS		0x2023
#define WGL_FULL_ACCELERATION	0x2027
#define WGL_SAMPLE_BUFFERS		0x2041
#define WGL_SAMPLES				0x2042

//=============================================================================================================
// Function used to choose a format that supports full screen anti-aliasing
//=============================================================================================================

typedef int (WINAPI *PFNWGLCHOOSEPIXELFORMATARBPROC) (HDC hdc,
	const int*		piAttribIList,
	const float*	pfAttribFList,
	unsigned int	nMaxFormats,
	int*			piFormats,
	unsigned int*	nNumFormats);

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormat = 0;

//=============================================================================================================
// Helper function that retrieves a pixel format that supports FSAA
//=============================================================================================================

int ChoosePixelFormat (HDC hDC, const PIXELFORMATDESCRIPTOR* pfd, uint aaLevel)
{
	if (aaLevel > 0)
	{
		// 2xFSAA, 4xFSAA, 8xFSAA, 16xFSAA
		if (aaLevel > 4) aaLevel = 4;

		WNDCLASS dummyClass;
		memset(&dummyClass, 0, sizeof(WNDCLASS));

		dummyClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		dummyClass.lpfnWndProc = (WNDPROC)DefWindowProc;
		dummyClass.lpszClassName = "OpenGL ChoosePixelFormat";

		// Register the class that will be used to create a dummy OpenGL window
		::RegisterClass(&dummyClass);

		// Create a dummy window
		HWND dummyWnd = ::CreateWindow(dummyClass.lpszClassName, dummyClass.lpszClassName,
			0, 0, 0, 1, 1, 0, 0, 0, 0);

		// Get the DC from the dummy window
		HDC dummyDC = ::GetDC(dummyWnd);

		// Choose a pixel format based on the descriptor above
		int pf = ::ChoosePixelFormat(dummyDC, pfd);

		// Activate it
		if ( ::SetPixelFormat(dummyDC, pf, pfd) != 0 )
		{
			// Create the OpenGL rendering context
			HGLRC dummyRC = ::wglCreateContext(dummyDC);

			if (dummyRC != 0)
			{
				// Make it current
				::wglMakeCurrent(dummyDC, dummyRC);

				// Find the OpenGL function used to choose a proper pixel format
				if (wglChoosePixelFormat == 0)
				{
					wglChoosePixelFormat = (PFNWGLCHOOSEPIXELFORMATARBPROC)::wglGetProcAddress("wglChoosePixelFormatARB");
					ASSERT(wglChoosePixelFormat != 0, "wglChoosePixelFormat seems to be missing!");
				}

				if (wglChoosePixelFormat)
				{
					unsigned int count (0);
					float fAttributes[] = {0, 0};
					int   iAttributes[] = {	WGL_DRAW_TO_WINDOW,		1,
											WGL_SUPPORT_OPENGL,		1,
											WGL_ACCELERATION,		WGL_FULL_ACCELERATION,
											WGL_RED_BITS,			8,
											WGL_GREEN_BITS,			8,
											WGL_BLUE_BITS,			8,
											WGL_ALPHA_BITS,			8,
											WGL_DEPTH_BITS,			pfd->cDepthBits,
											WGL_STENCIL_BITS,		pfd->cStencilBits,
											WGL_DOUBLE_BUFFER,		1,
											WGL_SAMPLE_BUFFERS,		1,
											WGL_SAMPLES,			1 << aaLevel,
											0,						0};

					// Ask OpenGL to choose a more appropriate pixel format
					wglChoosePixelFormat( dummyDC, iAttributes, fAttributes, 1, &pf, &count );
				}

				// Release the rendering context
				::wglMakeCurrent(0, 0);
				::wglDeleteContext(dummyRC);
			}
		}

		// Release everything and return the new pixel format
		::ReleaseDC(dummyWnd, dummyDC);
		::DestroyWindow(dummyWnd);
		::UnregisterClass("Dummy", 0);
		return pf;
	}
	return ::ChoosePixelFormat(hDC, pfd);
}

//=============================================================================================================

bool GLWindow::_CreateContext()
{
	bool retVal (false);

	if (mHRC == 0)
	{
		PIXELFORMATDESCRIPTOR pfd = {0};
		pfd.nSize			= sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion		= 1;
		pfd.dwFlags			= PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL;
		pfd.dwLayerMask		= PFD_MAIN_PLANE;
		pfd.iPixelType		= PFD_TYPE_RGBA;
		pfd.cColorBits		= 24;
		pfd.cAlphaBits		= 8;
		pfd.cDepthBits		= 24;
		pfd.cAccumBits		= 0;
		pfd.cStencilBits	= 8;

		int pf = ::ChoosePixelFormat(mHDC, &pfd, 1);
		::SetPixelFormat(mHDC, pf, &pfd);

		mGraphicsThread = Thread::GetID();
		mHRC = ::wglCreateContext(mHDC);
		ASSERT( mHRC != 0, "Failed to create the rendering context!" );

		if (mHRC)
		{
#ifdef _DEBUG
			System::Log("[WINDOW]  Created the rendering context");
			System::Log("          - Thread ID: %ld", mGraphicsThread);
#endif
			::wglMakeCurrent( mHDC, (HGLRC)mHRC );

			if ( mGraphics->Init() )
			{
				mGraphics->SetViewport(mSize);
				retVal = true;
			}
		}
	}
	return retVal;
}

//=============================================================================================================

void GLWindow::_ReleaseContext()
{
	if (mHRC != 0)
	{
		ASSERT(mGraphicsThread == Thread::GetID(),
			"You must call IWindow::SetGraphics from the same thread you called it the first time");

		::wglMakeCurrent(0, 0);
		::wglDeleteContext((HGLRC)mHRC);

		mGraphicsThread = 0;
		mGraphics = 0;
		mHRC = 0;
	}
}

//=============================================================================================================

void GLWindow::BeginFrame()
{
	if (mHRC != 0)
	{
		Lock();
		::wglMakeCurrent( mHDC, (HGLRC)mHRC);
	}
}

//=============================================================================================================

void GLWindow::EndFrame()
{
	if (mHDC != 0)
	{
		::SwapBuffers(mHDC);
		Unlock();
	}
}

#endif // _WINDOWS