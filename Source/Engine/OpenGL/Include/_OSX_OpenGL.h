#pragma once

//============================================================================================================
//			R5 Game Engine, individual file copyright belongs to their respective authors.
//									http://r5ge.googlecode.com/
//============================================================================================================
// Mac OSX header file for the OpenGL related functionality
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _MACOS

#ifndef _R5_OPENGL_OSX_H
#define _R5_OPENGL_OSX_H

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>
//#include "_glext.h"

#define glDeleteFramebuffers		glDeleteFramebuffersEXT
#define glGenFramebuffers			glGenFramebuffersEXT
#define glBindFramebuffer			glBindFramebufferEXT
#define glFramebufferTexture		glFramebufferTextureEXT
#define glFramebufferTexture2D		glFramebufferTexture2DEXT
#define glCheckFramebufferStatus	glCheckFramebufferStatusEXT
#define glFramebufferRenderbuffer	glFramebufferRenderbufferEXT
#define glGenRenderbuffers			glGenRenderbuffersEXT
#define glBindRenderbuffer			glBindRenderbufferEXT
#define glRenderbufferStorage		glRenderbufferStorageEXT
#define glDeleteRenderbuffers		glDeleteRenderbuffersEXT
#define glGenerateMipmap			glGenerateMipmapEXT
#define glBlitFramebuffer			glBlitFramebufferEXT
#define glGetObjectParameteri		glGetShaderiv
#define glGetInfoLog				glGetShaderInfoLog

// This extension doesn't seem to exist in my mac's <OpenGL/glext.h>
//#define glTexImage2DMultisample glTexImage2DMultisampleEXT
//#ifndef GL_TEXTURE_2D_MULTISAMPLE
//#define GL_TEXTURE_2D_MULTISAMPLE 0x9100
//#endif

#ifndef GL_FRAMEBUFFER
#define GL_FRAMEBUFFER 0x8D40
#endif

#ifndef GL_READ_FRAMEBUFFER
#define GL_READ_FRAMEBUFFER 0x8CA8
#endif

#ifndef GL_DRAW_FRAMEBUFFER
#define GL_DRAW_FRAMEBUFFER 0x8CA9
#endif

#ifndef GL_GEOMETRY_SHADER
#define GL_GEOMETRY_SHADER 0x8DD9
#endif

#ifndef GL_DEPTH_STENCIL
#define GL_DEPTH_STENCIL 0x84F9
#endif

#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_UNSIGNED_INT_24_8
#define GL_UNSIGNED_INT_24_8 0x84FA
#endif

#endif // _R5_OPENGL_OSX_H
#endif // _MACOS
