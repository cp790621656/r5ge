#include "../Include/_All.h"
#include "../Include/_OpenGL.h"
#include <GL/glx.h>

PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = 0;

bool _BindFunctionPointers()
{
#ifdef GL_ARB_texture_multisample
	glTexImage2DMultisample = 
		(PFNGLTEXIMAGE2DMULTISAMPLEPROC) glXGetProcAddress((const GLubyte*) "glTexImage2DMultisample");
	if (glTexImage2DMultisample == NULL) return false;
#endif
	return true;
}
