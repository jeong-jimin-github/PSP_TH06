#include "GLFunc.hpp"

#include <SDL2/SDL_video.h>

GLFuncTable g_glFuncTable;

#define TRY_RESOLVE_FUNCTION(func) this->func = (decltype(this->func))SDL_GL_GetProcAddress(#func);
#define TRY_RESOLVE_FUNCTION_GLES(func) this->func##_ptr = (decltype(this->func##_ptr))SDL_GL_GetProcAddress(#func);

void GLFuncTable::ResolveFunctions(bool glesContext)
{
#ifdef __PSP__
    // PSPGL exposes the OpenGL 1.x entry points as normal link-time symbols.
    // Its eglGetProcAddress implementation only resolves extensions, so using
    // SDL_GL_GetProcAddress for core functions leaves this table full of null
    // pointers on real hardware and PPSSPP.
#define DIRECT_RESOLVE_FUNCTION(func) this->func = &::func;
    DIRECT_RESOLVE_FUNCTION(glAlphaFunc)
    DIRECT_RESOLVE_FUNCTION(glBindTexture)
    DIRECT_RESOLVE_FUNCTION(glBlendFunc)
    DIRECT_RESOLVE_FUNCTION(glClear)
    DIRECT_RESOLVE_FUNCTION(glClearColor)
    DIRECT_RESOLVE_FUNCTION(glColorPointer)
    DIRECT_RESOLVE_FUNCTION(glDeleteTextures)
    DIRECT_RESOLVE_FUNCTION(glDepthFunc)
    DIRECT_RESOLVE_FUNCTION(glDepthMask)
    DIRECT_RESOLVE_FUNCTION(glDisableClientState)
    DIRECT_RESOLVE_FUNCTION(glDrawArrays)
    DIRECT_RESOLVE_FUNCTION(glEnable)
    DIRECT_RESOLVE_FUNCTION(glEnableClientState)
    DIRECT_RESOLVE_FUNCTION(glFogf)
    DIRECT_RESOLVE_FUNCTION(glFogfv)
    DIRECT_RESOLVE_FUNCTION(glGenTextures)
    DIRECT_RESOLVE_FUNCTION(glGetError)
    DIRECT_RESOLVE_FUNCTION(glGetFloatv)
    DIRECT_RESOLVE_FUNCTION(glGetIntegerv)
    DIRECT_RESOLVE_FUNCTION(glLoadIdentity)
    DIRECT_RESOLVE_FUNCTION(glLoadMatrixf)
    DIRECT_RESOLVE_FUNCTION(glMatrixMode)
    DIRECT_RESOLVE_FUNCTION(glMultMatrixf)
    DIRECT_RESOLVE_FUNCTION(glPopMatrix)
    DIRECT_RESOLVE_FUNCTION(glPushMatrix)
    DIRECT_RESOLVE_FUNCTION(glReadPixels)
    DIRECT_RESOLVE_FUNCTION(glShadeModel)
    DIRECT_RESOLVE_FUNCTION(glTexCoordPointer)
    DIRECT_RESOLVE_FUNCTION(glTexEnvfv)
    DIRECT_RESOLVE_FUNCTION(glTexEnvi)
    DIRECT_RESOLVE_FUNCTION(glTexImage2D)
    DIRECT_RESOLVE_FUNCTION(glTexParameteri)
    DIRECT_RESOLVE_FUNCTION(glTexSubImage2D)
    DIRECT_RESOLVE_FUNCTION(glVertexPointer)
    DIRECT_RESOLVE_FUNCTION(glViewport)
#undef DIRECT_RESOLVE_FUNCTION

    this->glClearDepth = &::glClearDepth;
    this->glDepthRange = &::glDepthRange;
    this->isGlesContext = false;
    return;
#endif

    TRY_RESOLVE_FUNCTION(glAlphaFunc)
    TRY_RESOLVE_FUNCTION(glBindTexture)
    TRY_RESOLVE_FUNCTION(glBlendFunc)
    TRY_RESOLVE_FUNCTION(glClear)
    TRY_RESOLVE_FUNCTION(glClearColor)
    TRY_RESOLVE_FUNCTION(glColorPointer)
    TRY_RESOLVE_FUNCTION(glDeleteTextures)
    TRY_RESOLVE_FUNCTION(glDepthFunc)
    TRY_RESOLVE_FUNCTION(glDepthMask)
    TRY_RESOLVE_FUNCTION(glDisableClientState)
    TRY_RESOLVE_FUNCTION(glDrawArrays)
    TRY_RESOLVE_FUNCTION(glEnable)
    TRY_RESOLVE_FUNCTION(glEnableClientState)
    TRY_RESOLVE_FUNCTION(glFogf)
    TRY_RESOLVE_FUNCTION(glFogfv)
    TRY_RESOLVE_FUNCTION(glGenTextures)
    TRY_RESOLVE_FUNCTION(glGetError)
    TRY_RESOLVE_FUNCTION(glGetFloatv)
    TRY_RESOLVE_FUNCTION(glGetIntegerv)
    TRY_RESOLVE_FUNCTION(glLoadIdentity)
    TRY_RESOLVE_FUNCTION(glLoadMatrixf)
    TRY_RESOLVE_FUNCTION(glMatrixMode)
    TRY_RESOLVE_FUNCTION(glMultMatrixf)
    TRY_RESOLVE_FUNCTION(glPopMatrix)
    TRY_RESOLVE_FUNCTION(glPushMatrix)
    TRY_RESOLVE_FUNCTION(glReadPixels)
    TRY_RESOLVE_FUNCTION(glShadeModel)
    TRY_RESOLVE_FUNCTION(glTexCoordPointer)
    TRY_RESOLVE_FUNCTION(glTexEnvfv)
    TRY_RESOLVE_FUNCTION(glTexEnvi)
    TRY_RESOLVE_FUNCTION(glTexImage2D)
    TRY_RESOLVE_FUNCTION(glTexParameteri)
    TRY_RESOLVE_FUNCTION(glTexSubImage2D)
    TRY_RESOLVE_FUNCTION(glVertexPointer)
    TRY_RESOLVE_FUNCTION(glViewport)

    // Ideally, we'd just check for both the regular GL and GLES version of the function and
    //   use whichever doesn't return NULL, but function resolves on GLX are actually context
    //   independent, meaning we can get a valid function pointer that then throws an error
    //   when we call it because the context doesn't actually match what's needed. So instead
    //   we need to pass a parameter to identify which function version to resolve and use.

    if (glesContext)
    {
        TRY_RESOLVE_FUNCTION_GLES(glClearDepthf)
        TRY_RESOLVE_FUNCTION_GLES(glDepthRangef)
    }
    else
    {
        TRY_RESOLVE_FUNCTION(glClearDepth)
        TRY_RESOLVE_FUNCTION(glDepthRange)
    }

    TRY_RESOLVE_FUNCTION(glAttachShader)
    TRY_RESOLVE_FUNCTION(glBindAttribLocation)
    TRY_RESOLVE_FUNCTION(glCompileShader)
    TRY_RESOLVE_FUNCTION(glCreateProgram)
    TRY_RESOLVE_FUNCTION(glCreateShader)
    TRY_RESOLVE_FUNCTION(glDeleteProgram)
    TRY_RESOLVE_FUNCTION(glDeleteShader)
    TRY_RESOLVE_FUNCTION(glDisableVertexAttribArray)
    TRY_RESOLVE_FUNCTION(glEnableVertexAttribArray)
    TRY_RESOLVE_FUNCTION(glGetProgramInfoLog)
    TRY_RESOLVE_FUNCTION(glGetProgramiv)
    TRY_RESOLVE_FUNCTION(glGetShaderInfoLog)
    TRY_RESOLVE_FUNCTION(glGetShaderiv)
    TRY_RESOLVE_FUNCTION(glGetUniformLocation)
    TRY_RESOLVE_FUNCTION(glLinkProgram)
    TRY_RESOLVE_FUNCTION(glShaderSource)
    TRY_RESOLVE_FUNCTION(glUniform1f)
    TRY_RESOLVE_FUNCTION(glUniform1i)
    TRY_RESOLVE_FUNCTION(glUniform4f)
    TRY_RESOLVE_FUNCTION(glUniformMatrix4fv)
    TRY_RESOLVE_FUNCTION(glUseProgram)
    TRY_RESOLVE_FUNCTION(glVertexAttribPointer)

    this->isGlesContext = glesContext;
}

void GLFuncTable::glClearDepthf(GLclampf depth)
{
    if (this->isGlesContext)
    {
        this->glClearDepthf_ptr(depth);
    }
    else
    {
        this->glClearDepth(depth);
    }
}

void GLFuncTable::glDepthRangef(GLclampf near_val, GLclampf far_val)
{
    if (this->isGlesContext)
    {
        this->glDepthRangef_ptr(near_val, far_val);
    }
    else
    {
        this->glDepthRange(near_val, far_val);
    }
}
