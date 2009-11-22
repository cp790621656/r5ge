#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

//=============================================================================================================
#if defined (_WINDOWS)
//=============================================================================================================

#include <windows.h>
using namespace R5;

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

		int pf = ::ChoosePixelFormat(mHDC, &pfd);
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