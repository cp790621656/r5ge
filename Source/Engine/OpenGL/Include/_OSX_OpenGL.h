#pragma once

//============================================================================================================
//											R5 Game Engine
//                                  Contact: arenmook@gmail.com
//============================================================================================================
// Mac OSX header file for the OpenGL related functionality
// Author: Michael Lyashenko
//============================================================================================================

#ifdef _MACOS

#ifndef _R5_OPENGL_OSX_H
#define _R5_OPENGL_OSX_H

#include <OpenGL/gl.h>
#include <OpenGL/glext.h>

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

#define glActiveClientTexture		glClientActiveTexture
#define glGetObjectParameteri		glGetShaderiv
#define glGetInfoLog				glGetShaderInfoLog

#endif // _R5_OPENGL_OSX_H
#endif // _MACOS