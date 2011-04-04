#pragma once

//============================================================================================================
//                  R5 Engine, Copyright (c) 2007-2011 Michael Lyashenko. All rights reserved.
//											www.nextrevision.com
//============================================================================================================
// MS Windows header file for the OpenGL related functionality
//============================================================================================================

#ifdef _WINDOWS

#ifndef _R5_OPENGL_WINDOWS_H
#define _R5_OPENGL_WINDOWS_H

//============================================================================================================
// Disables "macro redefinition" warning
//============================================================================================================

#pragma warning(disable: 4005)
#define	_WIN32_WINNT 0x0502

//============================================================================================================
// Common "windows.h" definitions -- out here so "windows.h" doesn't need to be included
//============================================================================================================

typedef struct HDC__*		HDC;
typedef struct HWND__*		HWND;
typedef struct HGLRC__*		HGLRC;
typedef struct HINSTANCE__* HINSTANCE;

#define WINGDIAPI	__declspec(dllimport)
#define APIENTRY	__stdcall
#define WINAPI		__stdcall
#define CALLBACK    __stdcall
#define IMPORT		__declspec(dllimport)

typedef int (WINAPI *PFNWGLSWAPINTERVALEXTPROC) (int interval);

//============================================================================================================
// Common OpenGL include files
//============================================================================================================

//#define GL_GLEXT_PROTOTYPES
#include <gl\glu.h>
#include "_glext.h"

//============================================================================================================
// Basic functions
//============================================================================================================

extern PFNGLACTIVETEXTUREARBPROC			glActiveTexture;
extern PFNGLCLIENTACTIVETEXTUREARBPROC		glActiveClientTexture;
extern PFNGLLOCKARRAYSEXTPROC				glLockArrays;
extern PFNGLUNLOCKARRAYSEXTPROC				glUnlockArrays;
extern PFNGLMULTITEXCOORD1FARBPROC			glMultiTexCoord1f;
extern PFNGLMULTITEXCOORD2FARBPROC			glMultiTexCoord2f;
extern PFNGLMULTITEXCOORD3FVARBPROC			glMultiTexCoord3fv;
extern PFNGLMULTITEXCOORD4FVARBPROC			glMultiTexCoord4fv;
extern PFNGLGENERATEMIPMAPEXTPROC			glGenerateMipmap;
extern PFNGLTEXIMAGE3DEXTPROC				glTexImage3D;

//============================================================================================================
// Open GL query functions (GL version 1.5+)
//============================================================================================================

extern PFNGLGENQUERIESARBPROC				glGenQueries;
extern PFNGLBEGINQUERYARBPROC				glBeginQuery;
extern PFNGLENDQUERYARBPROC					glEndQuery;
extern PFNGLGETQUERYOBJECTUIVARBPROC		glGetQueryObjectuiv;
extern PFNGLDELETEQUERIESARBPROC			glDeleteQueries;

//============================================================================================================
// Vertex Buffer Object (VBO) related functions
//============================================================================================================

extern PFNGLBINDBUFFERPROC					glBindBuffer;
extern PFNGLGENBUFFERSPROC					glGenBuffers;
extern PFNGLBUFFERDATAPROC					glBufferData;
extern PFNGLBUFFERSUBDATAPROC				glBufferSubData;
extern PFNGLDELETEBUFFERSPROC				glDeleteBuffers;
extern PFNGLMAPBUFFERPROC					glMapBuffer;
extern PFNGLUNMAPBUFFERPROC					glUnmapBuffer;

//============================================================================================================
// Frame buffer object related
//============================================================================================================

extern PFNGLGENFRAMEBUFFERSEXTPROC			glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFEREXTPROC			glBindFramebuffer;
//extern PFNGLFRAMEBUFFERTEXTUREEXTPROC		glFramebufferTexture;
extern PFNGLFRAMEBUFFERTEXTURE2DEXTPROC		glFramebufferTexture2D;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRender;
extern PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	glCheckFramebufferStatus;
extern PFNGLDELETEFRAMEBUFFERSEXTPROC		glDeleteFramebuffers;
extern PFNGLDRAWBUFFERSPROC					glDrawBuffers;
extern PFNGLGENRENDERBUFFERSEXTPROC			glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFEREXTPROC			glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEEXTPROC		glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRenderbuffer;
extern PFNGLDELETERENDERBUFFERSEXTPROC		glDeleteRenderbuffers;
extern PFNGLBLITFRAMEBUFFERPROC				glBlitFramebuffer;

//============================================================================================================
// Vertex and pixel shader related functions
//============================================================================================================

extern PFNGLCREATESHADEROBJECTARBPROC		glCreateShader;
extern PFNGLSHADERSOURCEARBPROC				glShaderSource;
extern PFNGLCOMPILESHADERARBPROC			glCompileShader;
extern PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgram;
extern PFNGLATTACHOBJECTARBPROC				glAttachShader;
extern PFNGLLINKPROGRAMARBPROC				glLinkProgram;
extern PFNGLUSEPROGRAMOBJECTARBPROC			glUseProgram;
extern PFNGLDETACHOBJECTARBPROC				glDetachShader;
extern PFNGLDELETEOBJECTARBPROC				glDeleteShader;
extern PFNGLGETOBJECTPARAMETERIVARBPROC		glGetObjectParameteri;
extern PFNGLGETINFOLOGARBPROC				glGetInfoLog;

//============================================================================================================
// Shader interaction
//============================================================================================================

extern PFNGLGETUNIFORMLOCATIONARBPROC		glGetUniformLocation;
extern PFNGLUNIFORM1FARBPROC				glUniform1f;
extern PFNGLUNIFORM2FARBPROC				glUniform2f;
extern PFNGLUNIFORM3FARBPROC				glUniform3f;
extern PFNGLUNIFORM4FARBPROC				glUniform4f;
extern PFNGLUNIFORM1IARBPROC				glUniform1i;
extern PFNGLUNIFORM2IARBPROC				glUniform2i;
extern PFNGLUNIFORM3IARBPROC				glUniform3i;
extern PFNGLUNIFORM4IARBPROC				glUniform4i;
extern PFNGLUNIFORM1FVARBPROC				glUniform1fv;
extern PFNGLUNIFORM2FVARBPROC				glUniform2fv;
extern PFNGLUNIFORM3FVARBPROC				glUniform3fv;
extern PFNGLUNIFORM4FVARBPROC				glUniform4fv;
extern PFNGLUNIFORM1IVARBPROC				glUniform1iv;
extern PFNGLUNIFORMMATRIX3FVARBPROC			glUniformMatrix3fv;
extern PFNGLUNIFORMMATRIX4FVARBPROC			glUniformMatrix4fv;
extern PFNGLVERTEXATTRIBPOINTERARBPROC		glVertexAttribPointer;
extern PFNGLGETVERTEXATTRIBPOINTERVARBPROC	glGetVertexAttribPointerv;
extern PFNGLGETATTRIBLOCATIONARBPROC		glGetAttribLocation;
extern PFNGLBINDATTRIBLOCATIONARBPROC		glBindAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYARBPROC	glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	glDisableVertexAttribArray;

//============================================================================================================
// MSAA extensions
//============================================================================================================

extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;

//============================================================================================================
// Windows-specific
//============================================================================================================

extern PFNWGLSWAPINTERVALEXTPROC glSwapInterval;

//============================================================================================================

#define glDeleteProgram 		glDeleteShader
#define glGetShaderiv			glGetObjectParameteri
#define glGetProgramiv			glGetObjectParameteri
#define glGetShaderInfoLog		glGetInfoLog
#define glGetProgramInfoLog		glGetInfoLog

// Bind all extensions
bool _BindFunctionPointers();

#endif // _R5_OPENGL_WINDOWS_H
#endif // _WINDOWS