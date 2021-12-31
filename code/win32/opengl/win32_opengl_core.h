/* date = December 29th 2021 4:19 pm */

#ifndef WIN32_OPENGL_CORE_H
#define WIN32_OPENGL_CORE_H

#include "win32_opengl_type.h"
#include "win32_opengl_debug.h"
#include "win32_opengl_ext.h"
#include "win32_opengl_context.h"
#include "win32_opengl_mesh.h"
#include "win32_opengl_buffer.h"
#include "win32_opengl_texture.h"
#include "win32_opengl_shader.h"
#include "win32_opengl_renderpass.h"

internal void
opengl_init(opengl_context *gl, u32 x, u32 y, u32 w, u32 h, char *title, memory_arena *arena)
{
    opengl_prepare_modern_context(gl);
    
    gl->hwnd = win32_make_window(x, y, w, h, title);
    gl->windowWidth = w;
    gl->windowHeight = h;
    
    gl->angle = 0;
    
    gl->hdc = GetDC(gl->hwnd);
    
    opengl_make_modern_context(gl);
    
    opengl_make_shaders(gl);
    
    opengl_set_global_state(gl);
    
    // Enable vsync
    gl->wglSwapIntervalEXT(1);
    
    win32_show_window(gl->hwnd);
}

internal void
opengl_prepare_frame(opengl_context *gl)
{
    // Setup output size covering all client area of window
    glViewport(0, 0, gl->windowWidth, gl->windowHeight);
    
    // clear screen
    glClearColor(0.392f, 0.584f, 0.929f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

internal void
opengl_draw_frame(opengl_context *gl, opengl_renderpass *renderPasses, u32 count)
{
    opengl_prepare_frame(gl);
    
    for (u32 i = 0;
         i < count;
         ++i)
    {
        opengl_execute_renderpass(gl, renderPasses[i]);
    }
    
    // Swap the buffers to show output
    if (!SwapBuffers(gl->hdc))
    {
        fatal_error("Failed to swap OpenGL buffers!");
    }
}

internal void
opengl_cleanup(opengl_context *gl)
{
    
}

#endif //WIN32_OPENGL_CORE_H
