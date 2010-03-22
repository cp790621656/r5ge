#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

//=============================================================================================================
#if defined (_MACOS)
//=============================================================================================================

#import <Cocoa/Cocoa.h>
using namespace R5;

//=============================================================================================================

bool GLWindow::_CreateContext()
{
	if (mGraphics != 0 && mContext == nil)
	{
		NSOpenGLPixelFormatAttribute attribs[20];
		UInt i = 0;

		attribs[i++] = NSOpenGLPFANoRecovery;
		attribs[i++] = NSOpenGLPFASingleRenderer;
		attribs[i++] = NSOpenGLPFAAccelerated;
		attribs[i++] = NSOpenGLPFADoubleBuffer;
		
		attribs[i++] = NSOpenGLPFAColorSize;
		attribs[i++] = (NSOpenGLPixelFormatAttribute) 24;

		attribs[i++] = NSOpenGLPFAAlphaSize;
		attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;
		
		attribs[i++] = NSOpenGLPFADepthSize;
		attribs[i++] = (NSOpenGLPixelFormatAttribute) 24;

		attribs[i++] = NSOpenGLPFAStencilSize;
		attribs[i++] = (NSOpenGLPixelFormatAttribute) 8;

		attribs[i++] = NSOpenGLPFAAccumSize;
		attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;

		if (mFullScreen)
		{
			attribs[i++] = NSOpenGLPFAFullScreen;
			attribs[i++] = NSOpenGLPFAScreenMask;
			attribs[i++] = (NSOpenGLPixelFormatAttribute) CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
		}

		//if (fsaa > 0)
		//{
		//	attribs[i++] = NSOpenGLPFASampleBuffers;
		//	attribs[i++] = (NSOpenGLPixelFormatAttribute) 1;
		//	attribs[i++] = NSOpenGLPFASamples;
		//	attribs[i++] = (NSOpenGLPixelFormatAttribute) fsaa;
		//}

		attribs[i++] = (NSOpenGLPixelFormatAttribute) 0;

		NSOpenGLPixelFormat* pxf = [[[NSOpenGLPixelFormat alloc] initWithAttributes: attribs] autorelease];

		if (pxf != nil)
		{
			mContext = [[NSOpenGLContext alloc] initWithFormat:pxf shareContext:nil];

			if (mContext != nil)
			{
				[mContext setView: (NSView*)mView];
				[mContext makeCurrentContext];
				return mGraphics->Init();
			}
		}
	}
	return false;
}

//=============================================================================================================

void GLWindow::_UpdateContext()
{
	if (mContext != nil) [mContext update];
}

//=============================================================================================================

void GLWindow::_ReleaseContext()
{
	if (mGraphics != 0)
	{
		if (mContext != nil)
		{
			[mContext makeCurrentContext];
			mGraphics->Release();
			mContext = nil;
		}
	}
}

//=============================================================================================================

void GLWindow::BeginFrame()
{
	if (mContext != nil)
	{
		Lock();
		[mContext makeCurrentContext];
	}
}

//=============================================================================================================

void GLWindow::EndFrame()
{
	if (mContext != nil)
	{
		[mContext flushBuffer];
		Unlock();
	}
}

#endif // _MACOS