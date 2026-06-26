#include "FixedFunctionGL.hpp"
#include "GameWindow.hpp"
#include "Supervisor.hpp"
#include "i18n.hpp"
#include <SDL2/SDL.h>

void FixedFunctionGL::SetContextFlags()
{
#ifdef __PSP__
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#endif
}

GfxInterface *FixedFunctionGL::Init()
{
    SetContextFlags();

    SDL_Init(SDL_INIT_VIDEO);

    u32 flags = SDL_WINDOW_OPENGL;
    i32 height = GAME_WINDOW_HEIGHT_REAL;
    i32 width = GAME_WINDOW_WIDTH_REAL;
    i32 x = SDL_WINDOWPOS_UNDEFINED;
    i32 y = SDL_WINDOWPOS_UNDEFINED;

#ifdef __PSP__
    flags |= SDL_WINDOW_FULLSCREEN;
#else
    if (g_Supervisor.cfg.windowed == 0)
    {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
#endif
    FixedFunctionGL *self = new FixedFunctionGL();

    SDL_Window *window = SDL_CreateWindow(TH_WINDOW_TITLE, x, y, width, height, flags);
    self->window = window;
    if (window == NULL)
    {
        delete self;
        return NULL;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    self->glContext = glContext;
    if (glContext == NULL)
    {
        delete self;
        return NULL;
    }

    if (SDL_GL_MakeCurrent(window, glContext) != 0)
    {
        delete self;
        return NULL;
    }

#ifdef __PSP__
    // Real PSP LCD scanout tears visibly when buffers are exchanged
    // immediately. GameWindow skips its redundant wall-clock limiter on PSP,
    // so VBlank alone provides stable 60 Hz pacing without a moving seam.
    SDL_GL_SetSwapInterval(1);
#else
    SDL_GL_SetSwapInterval(1);
#endif

    g_glFuncTable.ResolveFunctions(
#ifdef __PSP__
        true
#else
        false
#endif
    );

#ifdef __PSP__
    // Clear both PSP framebuffers before constraining all subsequent drawing
    // and clears to the centered 320x240 game image. The untouched border is
    // therefore always true black instead of inheriting stage fog colors.
    ::glDisable(GL_SCISSOR_TEST);
    g_glFuncTable.glViewport(0, 0, GAME_WINDOW_WIDTH_REAL, GAME_WINDOW_HEIGHT_REAL);
    g_glFuncTable.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    for (int buffer = 0; buffer < 2; ++buffer)
    {
        g_glFuncTable.glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
    }
    ::glEnable(GL_SCISSOR_TEST);
    ::glScissor(VIEWPORT_OFF_X, VIEWPORT_OFF_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
#endif

    g_glFuncTable.glEnable(GL_TEXTURE_2D);
    g_glFuncTable.glEnableClientState(GL_VERTEX_ARRAY);

    g_glFuncTable.glEnable(GL_ALPHA_TEST);
    g_glFuncTable.glAlphaFunc(GL_GEQUAL, 4 / 255.0f);

    if (((g_Supervisor.cfg.opts >> GCOS_SUPPRESS_USE_OF_GOROUD_SHADING) & 1) == 1)
    {
        g_glFuncTable.glShadeModel(GL_FLAT);
    }

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_FOG) & 1) == 0)
    {
        g_glFuncTable.glEnable(GL_FOG);
    }

    g_glFuncTable.glFogf(GL_FOG_MODE, GL_LINEAR);

#ifdef __PSP__
    // PSPGL implements the five classic texture environment modes, but not
    // the desktop GL 1.3 COMBINE source/operand state.
    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#else
    g_glFuncTable.glFogf(GL_FOG_DENSITY, 1.0f);
    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);

    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    }
    else
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_CONSTANT);
    }
    else
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

    if (((g_Supervisor.cfg.opts >> GCOS_NO_COLOR_COMP) & 1) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    }
    else
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

    if (((g_Supervisor.cfg.opts >> GCOS_DONT_USE_VERTEX_BUF) & 1) == 0)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
    }
    else
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
    }

    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
#endif

    return self;
}

void FixedFunctionGL::Exit()
{
    if (this->glContext)
    {
        SDL_GL_DeleteContext(this->glContext);
        this->glContext = NULL;
    }
    if (this->window)
    {
        SDL_DestroyWindow(this->window);
        this->window = NULL;
    }
}

void FixedFunctionGL::SetFogRange(f32 nearPlane, f32 farPlane)
{
#ifdef __PSP__
    // Setting START while PSPGL's default END is still zero raises
    // GL_INVALID_VALUE. Establish the far plane first.
    g_glFuncTable.glFogf(GL_FOG_END, farPlane);
    g_glFuncTable.glFogf(GL_FOG_START, nearPlane);
#else
    g_glFuncTable.glFogf(GL_FOG_START, nearPlane);
    g_glFuncTable.glFogf(GL_FOG_END, farPlane);
#endif
}

void FixedFunctionGL::SetFogColor(ZunColor color)
{
    GLfloat normalizedFogColor[4] = {((color >> 16) & 0xFF) / 255.0f, ((color >> 8) & 0xFF) / 255.0f,
                                     (color & 0xFF) / 255.0f, ((color >> 24) & 0xFF) / 255.0f};

    g_glFuncTable.glFogfv(GL_FOG_COLOR, normalizedFogColor);
}

void FixedFunctionGL::ToggleVertexAttribute(u8 attr, bool enable)
{
    if (attr & VERTEX_ATTR_TEX_COORD)
    {
#ifdef __PSP__
        if (enable)
        {
            g_glFuncTable.glEnable(GL_TEXTURE_2D);
            g_glFuncTable.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        else
        {
            g_glFuncTable.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            ::glDisable(GL_TEXTURE_2D);
        }
#else
        // Arg 0 will be the texture is it's used, and diffuse otherwise. Arg 1 will always be diffuse
        if (enable)
        {
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            g_glFuncTable.glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        else
        {
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
            g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
            g_glFuncTable.glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
#endif
    }

    if (attr & VERTEX_ATTR_DIFFUSE)
    {
        if (enable)
        {
            g_glFuncTable.glEnableClientState(GL_COLOR_ARRAY);
        }
        else
        {
            g_glFuncTable.glDisableClientState(GL_COLOR_ARRAY);
        }
    }
}

void FixedFunctionGL::SetAttributePointer(VertexAttributeArrays attr, std::size_t stride, void *ptr)
{
    switch (attr)
    {
    case VERTEX_ARRAY_POSITION:
        g_glFuncTable.glVertexPointer(3, GL_FLOAT, stride, ptr);
        break;
    case VERTEX_ARRAY_TEX_COORD:
        g_glFuncTable.glTexCoordPointer(2, GL_FLOAT, stride, ptr);
        break;
    case VERTEX_ARRAY_DIFFUSE:
        g_glFuncTable.glColorPointer(4, GL_UNSIGNED_BYTE, stride, ptr);
        break;
    }
}

void FixedFunctionGL::SetColorOp(TextureOpComponent component, ColorOp op)
{
    const GLenum opEnums[3] = {GL_MODULATE, GL_ADD, GL_REPLACE};

    if (component > COMPONENT_ALPHA || op > COLOR_OP_REPLACE)
    {
        return;
    }

#ifdef __PSP__
    // The PSP GE has one texture function for RGB and alpha together. EoSD's
    // alpha operation is normally modulation, so choose the RGB operation and
    // leave alpha-only changes alone.
    if (component == COMPONENT_RGB)
    {
        g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, opEnums[op]);
    }
#else
    GLenum componentEnum = component == COMPONENT_ALPHA ? GL_COMBINE_ALPHA : GL_COMBINE_RGB;
    g_glFuncTable.glTexEnvi(GL_TEXTURE_ENV, componentEnum, opEnums[op]);
#endif
}

void FixedFunctionGL::SetTextureFactor(ZunColor factor)
{
#ifdef __PSP__
    // GL_MODULATE on PSPGL consumes the current primary color. This is the
    // hardware equivalent of the GL_CONSTANT combine input used on desktop.
    ::glColor4ub((factor >> 16) & 0xFF, (factor >> 8) & 0xFF, factor & 0xFF, (factor >> 24) & 0xFF);
#else
    GLfloat tfactorColor[4] = {((factor >> 16) & 0xFF) / 255.0f, ((factor >> 8) & 0xFF) / 255.0f,
                               (factor & 0xFF) / 255.0f, ((factor >> 24) & 0xFF) / 255.0f};

    g_glFuncTable.glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, tfactorColor);
#endif
}

void FixedFunctionGL::SetTransformMatrix(TransformMatrix type, const ZunMatrix &matrix)
{
    // This is not going to work for modelview
    GLenum matrixEnum[4] = {GL_MODELVIEW, GL_MODELVIEW, GL_PROJECTION, GL_TEXTURE};

    g_glFuncTable.glMatrixMode(matrixEnum[type]);
    g_glFuncTable.glLoadMatrixf((const GLfloat *)&matrix);
}

void FixedFunctionGL::Enable(Capabilities cap)
{
    switch (cap)
    {
    case CAPS_BLEND:
        g_glFuncTable.glEnable(GL_BLEND);
        break;
    case CAPS_DEPTH_TEST:
        g_glFuncTable.glEnable(GL_DEPTH_TEST);
        break;
    }
}

bool FixedFunctionGL::HasError()
{
    return g_glFuncTable.glGetError() != GL_NO_ERROR;
}

void FixedFunctionGL::SetBlendMode(BlendMode mode)
{
    if (mode == BLEND_INV_SRC_ALPHA)
    {
        g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        g_glFuncTable.glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
}

void FixedFunctionGL::SetViewport(i32 x, i32 y, i32 width, i32 height)
{
#ifdef __PSP__
    cachedViewport[0] = x;
    cachedViewport[1] = y;
    cachedViewport[2] = width;
    cachedViewport[3] = height;
#endif
    g_glFuncTable.glViewport(x, y, width, height);
}

void FixedFunctionGL::GetViewport(u32 *viewport)
{
#ifdef __PSP__
    for (u32 i = 0; i < 4; i++)
    {
        viewport[i] = cachedViewport[i];
    }
#else
    g_glFuncTable.glGetIntegerv(GL_VIEWPORT, (GLint *)viewport);
#endif
}

void FixedFunctionGL::GetDepthRange(f32 *depthRange)
{
#ifdef __PSP__
    depthRange[0] = cachedDepthRange[0];
    depthRange[1] = cachedDepthRange[1];
#else
    g_glFuncTable.glGetFloatv(GL_DEPTH_RANGE, depthRange);
#endif
}

void FixedFunctionGL::SetClearColor(f32 r, f32 g, f32 b, f32 a)
{
    g_glFuncTable.glClearColor(r, g, b, a);
}

void FixedFunctionGL::SetTextureFilter()
{
    g_glFuncTable.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void FixedFunctionGL::SetClearDepth(f32 depth)
{
    g_glFuncTable.glClearDepthf(depth);
}

void FixedFunctionGL::Clear(u32 clearBits)
{
    GLbitfield mask = 0;

    if (clearBits & CLEAR_COLOR_BUFFER)
        mask |= GL_COLOR_BUFFER_BIT;
    if (clearBits & CLEAR_DEPTH_BUFFER)
        mask |= GL_DEPTH_BUFFER_BIT;

    g_glFuncTable.glClear(mask);
}

void FixedFunctionGL::SetDepthRange(f32 nearPlane, f32 farPlane)
{
#ifdef __PSP__
    cachedDepthRange[0] = nearPlane;
    cachedDepthRange[1] = farPlane;
#endif
    g_glFuncTable.glDepthRangef(nearPlane, farPlane);
}

void FixedFunctionGL::SetDepthMask(bool enable)
{
    g_glFuncTable.glDepthMask(enable);
}

void FixedFunctionGL::SetDepthFunc(DepthFunc func)
{
    if (func == DEPTH_FUNC_ALWAYS)
    {
        g_glFuncTable.glDepthFunc(GL_ALWAYS);
    }
    else
    {
        g_glFuncTable.glDepthFunc(GL_LEQUAL);
    }
}

GfxTextureHandle FixedFunctionGL::CreateTexture()
{
    GLuint texture;
    g_glFuncTable.glGenTextures(1, &texture);

    return texture;
}

void FixedFunctionGL::BindTexture(GfxTextureHandle handle)
{
    g_glFuncTable.glBindTexture(GL_TEXTURE_2D, handle);
}

void FixedFunctionGL::DeleteTexture(GfxTextureHandle handle)
{
    g_glFuncTable.glDeleteTextures(1, (GLuint *)&handle);
}

void FixedFunctionGL::SetTextureImage(u32 width, u32 height, PixelFormat fmt, PixelDataType type, const void *data)
{
    GLenum glFmt;
    GLenum glType;

    switch (fmt)
    {
    case PIXEL_RGBA:
        glFmt = GL_RGBA;
        break;
    case PIXEL_RGB:
        glFmt = GL_RGB;
        break;
    }

    switch (type)
    {
    case PIXEL_UNSIGNED_BYTE:
        glType = GL_UNSIGNED_BYTE;
        break;
    case PIXEL_UNSIGNED_SHORT_5_5_5_1:
        glType = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
    case PIXEL_UNSIGNED_SHORT_5_6_5:
        glType = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case PIXEL_UNSIGNED_SHORT_4_4_4_4:
        glType = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
    }

    g_glFuncTable.glTexImage2D(GL_TEXTURE_2D, 0, glFmt, width, height, 0, glFmt, glType, data);
}

void FixedFunctionGL::SetTextureSubImage(i32 xoffset, i32 yoffset, i32 width, i32 height, const void *data)
{
    g_glFuncTable.glTexSubImage2D(GL_TEXTURE_2D, 0, xoffset, yoffset, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void FixedFunctionGL::ReadPixels(i32 x, i32 y, i32 width, i32 height, const void *pixels)
{
    g_glFuncTable.glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, (void *)pixels);
}

void FixedFunctionGL::Draw(PrimitiveType type, i32 start, i32 count)
{
    GLenum glPrim;

    switch (type)
    {
    case PRIM_TRIANGLE_STRIP:
        glPrim = GL_TRIANGLE_STRIP;
        break;
    case PRIM_TRIANGLES:
        glPrim = GL_TRIANGLES;
        break;
    }

    g_glFuncTable.glDrawArrays(glPrim, start, count);
}

void FixedFunctionGL::SwapBuffers()
{
    SDL_GL_SwapWindow(window);
}
