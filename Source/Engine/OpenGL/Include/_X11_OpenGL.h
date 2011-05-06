#ifdef _LINUX

#ifndef _R5_OPENGL_X11_H
#define _R5_OPENGL_X11_H

#include <GL/gl.h>
#include <GL/glext.h>

extern "C" 
{
extern void APIENTRY glEnableVertexAttribArray (GLuint index);
extern void APIENTRY glVertexAttribPointer (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
extern void APIENTRY glDisableVertexAttribArray (GLuint index);
extern void APIENTRY glVertexAttrib4f (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern GLenum APIENTRY glCheckFramebufferStatus (GLenum target);
extern void APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer);
extern void APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
extern void APIENTRY glGenFramebuffers (GLsizei n, GLuint *framebuffers);
extern void APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void APIENTRY glDrawBuffers (GLsizei n, const GLenum *bufs);
extern void APIENTRY glBlitFramebuffer (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
extern void APIENTRY glGenQueries (GLsizei n, GLuint *ids);
extern void APIENTRY glDeleteQueries (GLsizei n, const GLuint *ids);
extern void APIENTRY glBeginQuery (GLenum target, GLuint id);
extern void APIENTRY glEndQuery (GLenum target);
extern void APIENTRY glGetQueryObjectuiv (GLuint id, GLenum pname, GLuint *params);
extern void APIENTRY glDeleteShader (GLuint shader);
extern GLuint APIENTRY glCreateShader (GLenum type);
extern void APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar* *string, const GLint *length);
extern void APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint *params);
extern void APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void APIENTRY glUseProgram (GLuint program);
extern void APIENTRY glDetachShader (GLuint program, GLuint shader);
extern GLuint APIENTRY glCreateProgram (void);
extern void APIENTRY glAttachShader (GLuint program, GLuint shader);
extern void APIENTRY glLinkProgram (GLuint program);
extern GLint APIENTRY glGetUniformLocation (GLuint program, const GLchar *name);
extern void APIENTRY glUniform1i (GLint location, GLint v0);
extern void APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar *name);
extern void APIENTRY glUniform1f (GLint location, GLfloat v0);
extern void APIENTRY glUniform2f (GLint location, GLfloat v0, GLfloat v1);
extern void APIENTRY glUniform3f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void APIENTRY glUniform4f (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint *value);
extern void APIENTRY glGenerateMipmap (GLenum target);
extern void APIENTRY glBindBuffer (GLenum target, GLuint buffer);
extern void APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers);
extern void APIENTRY glGenBuffers (GLsizei n, GLuint *buffers);
extern GLvoid* APIENTRY glMapBuffer (GLenum target, GLenum access);
extern GLboolean APIENTRY glUnmapBuffer (GLenum target);
extern void APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
extern void APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern GLenum APIENTRY glCheckFramebufferStatus (GLenum target);
extern void APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint *renderbuffers);
extern void APIENTRY glCompileShader (GLuint shader);
extern void APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint *params);
extern void APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void APIENTRY glDeleteProgram (GLuint program);
extern void APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat *value);
extern void APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat *value);
extern void APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat *value);
extern void APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat *value);
}

extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;

bool _BindFunctionPointers();

#endif //_R5_OPENGL_X11_H
#endif //_LINUX
