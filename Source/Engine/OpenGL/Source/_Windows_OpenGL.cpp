#include "../Include/_All.h"
#include "../Include/_OpenGL.h"

#ifdef _WINDOWS
#include <windows.h>
#pragma comment(lib, "opengl32.lib")
using namespace R5;

//============================================================================================================
// Basic functions
//============================================================================================================

PFNGLACTIVETEXTUREARBPROC			glActiveTexture			= 0;
PFNGLCLIENTACTIVETEXTUREARBPROC		glActiveClientTexture	= 0;
PFNGLLOCKARRAYSEXTPROC				glLockArrays			= 0;
PFNGLUNLOCKARRAYSEXTPROC			glUnlockArrays			= 0;
PFNGLMULTITEXCOORD1FARBPROC			glMultiTexCoord1f		= 0;
PFNGLMULTITEXCOORD2FARBPROC			glMultiTexCoord2f		= 0;
PFNGLGENERATEMIPMAPEXTPROC			glGenerateMipmap		= 0;
PFNGLTEXIMAGE3DEXTPROC				glTexImage3D			= 0;

//============================================================================================================
// Open GL query functions (GL version 1.5+)
//============================================================================================================

PFNGLGENQUERIESARBPROC				glGenQueries			= 0;
PFNGLBEGINQUERYARBPROC				glBeginQuery			= 0;
PFNGLENDQUERYARBPROC				glEndQuery				= 0;
PFNGLGETQUERYOBJECTUIVARBPROC		glGetQueryObjectuiv		= 0;
PFNGLDELETEQUERIESARBPROC			glDeleteQueries			= 0;

//============================================================================================================
// Vertex Buffer Object (VBO) related functions
//============================================================================================================

PFNGLBINDBUFFERPROC					glBindBuffer			= 0;
PFNGLGENBUFFERSPROC					glGenBuffers			= 0;
PFNGLBUFFERDATAPROC					glBufferData			= 0;
PFNGLBUFFERSUBDATAPROC				glBufferSubData			= 0;
PFNGLDELETEBUFFERSPROC				glDeleteBuffers			= 0;
PFNGLMAPBUFFERPROC					glMapBuffer				= 0;
PFNGLUNMAPBUFFERPROC				glUnmapBuffer			= 0;

//============================================================================================================
// Frame buffer object related
//============================================================================================================

PFNGLGENFRAMEBUFFERSEXTPROC			glGenFramebuffers		= 0;
PFNGLBINDFRAMEBUFFEREXTPROC			glBindFramebuffer		= 0;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC	glFramebufferTexture2D	= 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRender		= 0;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC	glCheckFramebufferStatus= 0;
PFNGLDELETEFRAMEBUFFERSEXTPROC		glDeleteFramebuffers	= 0;
PFNGLDRAWBUFFERSPROC				glDrawBuffers			= 0;
PFNGLGENRENDERBUFFERSEXTPROC		glGenRenderbuffers		= 0;
PFNGLBINDRENDERBUFFEREXTPROC		glBindRenderbuffer		= 0;
PFNGLRENDERBUFFERSTORAGEEXTPROC		glRenderbufferStorage	= 0;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC	glFramebufferRenderbuffer = 0;
PFNGLDELETERENDERBUFFERSEXTPROC		glDeleteRenderbuffers	= 0;

//============================================================================================================
// Vertex and pixel shader related functions
//============================================================================================================

PFNGLCREATESHADEROBJECTARBPROC		glCreateShader			= 0;
PFNGLSHADERSOURCEARBPROC			glShaderSource			= 0;
PFNGLCOMPILESHADERARBPROC			glCompileShader			= 0;
PFNGLCREATEPROGRAMOBJECTARBPROC		glCreateProgram			= 0;
PFNGLATTACHOBJECTARBPROC			glAttachShader			= 0;
PFNGLLINKPROGRAMARBPROC				glLinkProgram			= 0;
PFNGLUSEPROGRAMOBJECTARBPROC		glUseProgram			= 0;
PFNGLDETACHOBJECTARBPROC			glDetachShader			= 0;
PFNGLDELETEOBJECTARBPROC			glDeleteProgram			= 0;
PFNGLGETOBJECTPARAMETERIVARBPROC	glGetObjectParameteri	= 0;
PFNGLGETINFOLOGARBPROC				glGetInfoLog			= 0;

//============================================================================================================
// GLShader interaction
//============================================================================================================

PFNGLGETUNIFORMLOCATIONARBPROC			glGetUniformLocation		= 0;
PFNGLUNIFORM1FARBPROC					glUniform1f					= 0;
PFNGLUNIFORM2FARBPROC					glUniform2f					= 0;
PFNGLUNIFORM3FARBPROC					glUniform3f					= 0;
PFNGLUNIFORM4FARBPROC					glUniform4f					= 0;
PFNGLUNIFORM1IARBPROC					glUniform1i					= 0;
PFNGLUNIFORM2IARBPROC					glUniform2i					= 0;
PFNGLUNIFORM3IARBPROC					glUniform3i					= 0;
PFNGLUNIFORM4IARBPROC					glUniform4i					= 0;
PFNGLUNIFORM1IVARBPROC					glUniform1iv				= 0;
PFNGLUNIFORM1FVARBPROC					glUniform1fv				= 0;
PFNGLUNIFORM2FVARBPROC					glUniform2fv				= 0;
PFNGLUNIFORM3FVARBPROC					glUniform3fv				= 0;
PFNGLUNIFORM4FVARBPROC					glUniform4fv				= 0;
PFNGLUNIFORMMATRIX3FVARBPROC			glUniformMatrix3fv			= 0;
PFNGLUNIFORMMATRIX4FVARBPROC			glUniformMatrix4fv			= 0;
PFNGLVERTEXATTRIBPOINTERARBPROC			glVertexAttribPointer		= 0;
PFNGLGETVERTEXATTRIBPOINTERVARBPROC		glGetVertexAttribPointerv	= 0;
PFNGLGETATTRIBLOCATIONARBPROC			glGetAttribLocation			= 0;
PFNGLBINDATTRIBLOCATIONARBPROC			glBindAttribLocation		= 0;
PFNGLENABLEVERTEXATTRIBARRAYARBPROC		glEnableVertexAttribArray	= 0;
PFNGLDISABLEVERTEXATTRIBARRAYARBPROC	glDisableVertexAttribArray	= 0;

//============================================================================================================
// Windows-specific
//============================================================================================================

PFNWGLSWAPINTERVALEXTPROC	glSwapInterval	= 0;

//============================================================================================================
// Slightly more descriptive version of "wglGetProcAddress"
//============================================================================================================

PROC glGetFunction(bool& flag, const char* name)
{
	PROC retVal = ::wglGetProcAddress(name);

	if (retVal == 0)
	{
		System::Log("[OPENGL]  ERROR! Function not supported: '%s'", name);
		flag = false;
	}

	return retVal;
}

//============================================================================================================
// Binds all function pointers
//============================================================================================================

bool _BindFunctionPointers()
{
	Vector2i version = ::GetVersion();
	byte minor = (version.x >> 8) & 0xF;
	byte major =  version.x & 0xF;

	if (major == 6)
	{
		// Vista and Windows7 get their own device entry
		g_caps.mOS = (minor == 0) ? DeviceInfo::OS::WindowsVista : DeviceInfo::OS::Windows7;
	}

	bool essential (true), secondary (true);

	glActiveTexture				= (PFNGLACTIVETEXTUREARBPROC)			glGetFunction(essential, "glActiveTextureARB");
	glActiveClientTexture		= (PFNGLCLIENTACTIVETEXTUREARBPROC)		glGetFunction(essential, "glClientActiveTextureARB");
	glLockArrays				= (PFNGLLOCKARRAYSEXTPROC)				glGetFunction(essential, "glLockArraysEXT");
	glUnlockArrays				= (PFNGLUNLOCKARRAYSEXTPROC)			glGetFunction(essential, "glUnlockArraysEXT");
	glMultiTexCoord1f			= (PFNGLMULTITEXCOORD1FARBPROC)			glGetFunction(essential, "glMultiTexCoord1fARB");
	glMultiTexCoord2f			= (PFNGLMULTITEXCOORD2FARBPROC)			glGetFunction(essential, "glMultiTexCoord2fARB");
	glGenerateMipmap			= (PFNGLGENERATEMIPMAPEXTPROC)			glGetFunction(essential, "glGenerateMipmapEXT");
	glTexImage3D				= (PFNGLTEXIMAGE3DEXTPROC)				glGetFunction(essential, "glTexImage3DEXT");
	
	glGenQueries				= (PFNGLGENQUERIESARBPROC)				glGetFunction(secondary, "glGenQueriesARB");
	glBeginQuery				= (PFNGLBEGINQUERYARBPROC)				glGetFunction(secondary, "glBeginQueryARB");
	glEndQuery					= (PFNGLENDQUERYARBPROC)				glGetFunction(secondary, "glEndQueryARB");
	glGetQueryObjectuiv			= (PFNGLGETQUERYOBJECTUIVARBPROC)		glGetFunction(secondary, "glGetQueryObjectuivARB");
	glDeleteQueries				= (PFNGLDELETEQUERIESARBPROC)			glGetFunction(secondary, "glDeleteQueriesARB");

	glBindBuffer				= (PFNGLBINDBUFFERPROC)					glGetFunction(essential, "glBindBuffer");
	glGenBuffers				= (PFNGLGENBUFFERSPROC)					glGetFunction(essential, "glGenBuffers");
	glBufferData				= (PFNGLBUFFERDATAPROC)					glGetFunction(essential, "glBufferData");
	glBufferSubData				= (PFNGLBUFFERSUBDATAPROC)				glGetFunction(essential, "glBufferSubData");
	glDeleteBuffers				= (PFNGLDELETEBUFFERSPROC)				glGetFunction(essential, "glDeleteBuffers");
	glMapBuffer					= (PFNGLMAPBUFFERPROC)					glGetFunction(essential, "glMapBuffer");
	glUnmapBuffer				= (PFNGLUNMAPBUFFERPROC)				glGetFunction(essential, "glUnmapBuffer");

	glGenFramebuffers			= (PFNGLGENFRAMEBUFFERSEXTPROC)			glGetFunction(essential, "glGenFramebuffersEXT");
	glBindFramebuffer			= (PFNGLBINDFRAMEBUFFEREXTPROC)			glGetFunction(essential, "glBindFramebufferEXT");
	glFramebufferTexture2D		= (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)	glGetFunction(essential, "glFramebufferTexture2DEXT");
	glFramebufferRender			= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) glGetFunction(essential, "glFramebufferRenderbufferEXT");
	glCheckFramebufferStatus	= (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)	glGetFunction(essential, "glCheckFramebufferStatusEXT");
	glDeleteFramebuffers		= (PFNGLDELETEFRAMEBUFFERSEXTPROC)		glGetFunction(essential, "glDeleteFramebuffersEXT");
	glDrawBuffers				= (PFNGLDRAWBUFFERSPROC)				glGetFunction(essential, "glDrawBuffers");
	glGenRenderbuffers			= (PFNGLGENRENDERBUFFERSEXTPROC)		glGetFunction(essential, "glGenRenderbuffersEXT");
	glBindRenderbuffer			= (PFNGLBINDRENDERBUFFEREXTPROC)		glGetFunction(essential, "glBindRenderbufferEXT");
	glRenderbufferStorage		= (PFNGLRENDERBUFFERSTORAGEEXTPROC)		glGetFunction(essential, "glRenderbufferStorageEXT");
	glFramebufferRenderbuffer	= (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)	glGetFunction(essential, "glFramebufferRenderbufferEXT");
	glDeleteRenderbuffers		= (PFNGLDELETERENDERBUFFERSEXTPROC)		glGetFunction(essential, "glDeleteRenderbuffersEXT");
	
	glCreateShader				= (PFNGLCREATESHADEROBJECTARBPROC)		glGetFunction(essential, "glCreateShaderObjectARB");
	glShaderSource				= (PFNGLSHADERSOURCEARBPROC)			glGetFunction(essential, "glShaderSourceARB");
	glCompileShader				= (PFNGLCOMPILESHADERARBPROC)			glGetFunction(essential, "glCompileShaderARB");
	glAttachShader				= (PFNGLATTACHOBJECTARBPROC)			glGetFunction(essential, "glAttachObjectARB");
	glCreateProgram				= (PFNGLCREATEPROGRAMOBJECTARBPROC)		glGetFunction(essential, "glCreateProgramObjectARB");
	glLinkProgram				= (PFNGLLINKPROGRAMARBPROC)				glGetFunction(essential, "glLinkProgramARB");
	glUseProgram				= (PFNGLUSEPROGRAMOBJECTARBPROC)		glGetFunction(essential, "glUseProgramObjectARB");
	glDetachShader				= (PFNGLDETACHOBJECTARBPROC)			glGetFunction(essential, "glDetachObjectARB");
	glDeleteProgram				= (PFNGLDELETEOBJECTARBPROC)			glGetFunction(essential, "glDeleteObjectARB");
	glGetObjectParameteri		= (PFNGLGETOBJECTPARAMETERIVARBPROC)	glGetFunction(essential, "glGetObjectParameterivARB");
	glGetInfoLog				= (PFNGLGETINFOLOGARBPROC)				glGetFunction(essential, "glGetInfoLogARB");
	glVertexAttribPointer		= (PFNGLVERTEXATTRIBPOINTERARBPROC)		glGetFunction(essential, "glVertexAttribPointerARB");
	glGetAttribLocation			= (PFNGLGETATTRIBLOCATIONARBPROC)		glGetFunction(essential, "glGetAttribLocationARB");
	glGetVertexAttribPointerv	= (PFNGLGETVERTEXATTRIBPOINTERVARBPROC)	glGetFunction(essential, "glGetVertexAttribPointervARB");
	glBindAttribLocation		= (PFNGLBINDATTRIBLOCATIONARBPROC)		glGetFunction(essential, "glBindAttribLocationARB");
	glEnableVertexAttribArray	= (PFNGLENABLEVERTEXATTRIBARRAYARBPROC)	glGetFunction(essential, "glEnableVertexAttribArrayARB");
	glDisableVertexAttribArray	= (PFNGLDISABLEVERTEXATTRIBARRAYARBPROC)glGetFunction(essential, "glDisableVertexAttribArrayARB");

	glGetUniformLocation		= (PFNGLGETUNIFORMLOCATIONARBPROC)		glGetFunction(essential, "glGetUniformLocationARB");
	glUniform1f					= (PFNGLUNIFORM1FARBPROC)				glGetFunction(essential, "glUniform1fARB");
	glUniform2f					= (PFNGLUNIFORM2FARBPROC)				glGetFunction(essential, "glUniform2fARB");
	glUniform3f					= (PFNGLUNIFORM3FARBPROC)				glGetFunction(essential, "glUniform3fARB");
	glUniform4f					= (PFNGLUNIFORM4FARBPROC)				glGetFunction(essential, "glUniform4fARB");
	glUniform1i					= (PFNGLUNIFORM1IARBPROC)				glGetFunction(essential, "glUniform1iARB");
	glUniform2i					= (PFNGLUNIFORM2IARBPROC)				glGetFunction(essential, "glUniform2iARB");
	glUniform3i					= (PFNGLUNIFORM3IARBPROC)				glGetFunction(essential, "glUniform3iARB");
	glUniform4i					= (PFNGLUNIFORM4IARBPROC)				glGetFunction(essential, "glUniform4iARB");
	glUniform1fv				= (PFNGLUNIFORM1FVARBPROC)				glGetFunction(essential, "glUniform1fvARB");
	glUniform2fv				= (PFNGLUNIFORM2FVARBPROC)				glGetFunction(essential, "glUniform2fvARB");
	glUniform3fv				= (PFNGLUNIFORM3FVARBPROC)				glGetFunction(essential, "glUniform3fvARB");
	glUniform4fv				= (PFNGLUNIFORM4FVARBPROC)				glGetFunction(essential, "glUniform4fvARB");
	glUniform1iv				= (PFNGLUNIFORM1IVARBPROC)				glGetFunction(essential, "glUniform1ivARB");
	glUniformMatrix3fv			= (PFNGLUNIFORMMATRIX3FVARBPROC)		glGetFunction(essential, "glUniformMatrix3fvARB");
	glUniformMatrix4fv			= (PFNGLUNIFORMMATRIX4FVARBPROC)		glGetFunction(essential, "glUniformMatrix4fvARB");

	glSwapInterval				= (PFNWGLSWAPINTERVALEXTPROC)			glGetFunction(secondary, "wglSwapIntervalEXT");

	return essential;
}

#endif // _WINDOWS