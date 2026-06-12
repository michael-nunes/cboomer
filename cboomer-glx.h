#ifndef __CBOOMER_GLX__
#define __CBOOMER_GLX__

#include <GL/glx.h>
#include <GL/gl.h>

#define WHAT_WE_NEED_GL \
	_X(PFNGLCREATESHADERPROC,           glCreateShader) \
	_X(PFNGLSHADERSOURCEPROC,           glShaderSource) \
	_X(PFNGLCREATEPROGRAMPROC,          glCreateProgram) \
	_X(PFNGLCOMPILESHADERPROC,          glCompileShader) \
	_X(PFNGLGETSHADERIVPROC,            glGetShaderiv) \
	_X(PFNGLGETSHADERINFOLOGPROC,       glGetShaderInfoLog) \
	_X(PFNGLATTACHSHADERPROC,           glAttachShader) \
	_X(PFNGLLINKPROGRAMPROC,            glLinkProgram) \
	_X(PFNGLDELETESHADERPROC,           glDeleteShader) \
	_X(PFNGLGETPROGRAMIVPROC,           glGetProgramiv) \
	_X(PFNGLGETPROGRAMINFOLOGPROC,      glGetProgramInfoLog) \
	_X(PFNGLUSEPROGRAMPROC,             glUseProgram) \
	_X(PFNGLGETUNIFORMLOCATIONPROC,     glGetUniformLocation) \
	_X(PFNGLUNIFORM1FPROC,              glUniform1f) \
	_X(PFNGLUNIFORM2FPROC,              glUniform2f) \
	_X(PFNGLUNIFORM1IPROC,              glUniform1i) \
	_X(PFNGLGENVERTEXARRAYSPROC,        glGenVertexArrays) \
	_X(PFNGLGENBUFFERSPROC,             glGenBuffers) \
	_X(PFNGLBINDVERTEXARRAYPROC,        glBindVertexArray) \
	_X(PFNGLBINDBUFFERPROC,             glBindBuffer) \
	_X(PFNGLBUFFERDATAPROC,             glBufferData) \
	_X(PFNGLGENERATEMIPMAPPROC,         glGenerateMipmap) \
	_X(PFNGLVERTEXATTRIBPOINTERPROC,    glVertexAttribPointer) \
	_X(PFNGLENABLEVERTEXATTRIBARRAYPROC,glEnableVertexAttribArray) \
	_X(PFNGLXGETMSCRATEOMLPROC,glXGetMscRateOML) 

#define _X(ftype, name) ftype name = NULL;
	WHAT_WE_NEED_GL
#undef _X

static void CBoomerLoadGLExtensions() {
		#define _X(ftype, name) \
		name = (ftype)glXGetProcAddressARB((const GLubyte*) #name);\
		if(name == NULL){ fprintf(stderr, "Cannot load %s function\n", #name); exit(1); } 
		WHAT_WE_NEED_GL
	#undef _X
}

#endif
