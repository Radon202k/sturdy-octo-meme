/* date = December 29th 2021 4:19 pm */

#ifndef WIN32_OPENGL_CORE_H
#define WIN32_OPENGL_CORE_H

#include "win32_opengl_type.h"
#include "win32_opengl_debug.h"
#include "win32_opengl_ext.h"
#include "win32_opengl_context.h"
#include "win32_opengl_buffer.h"
#include "win32_opengl_texture.h"
#include "win32_opengl_shader.h"

internal void
opengl_init(opengl_context *gl, u32 x, u32 y, u32 w, u32 h, char *title)
{
    opengl_prepare_modern_context(gl);
    
    gl->hwnd = win32_make_window(x, y, w, h, title);
    gl->windowWidth = w;
    gl->windowHeight = h;
    
    gl->angle = 0;
    
    gl->hdc = GetDC(gl->hwnd);
    
    opengl_make_modern_context(gl);
    
    opengl_make_buffers(gl);
    
    opengl_make_textures(gl);
    
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
opengl_draw_frame(opengl_context *gl)
{
    opengl_prepare_frame(gl);
    
    opengl_upload_uniform_buffer_data(gl);
    
    // Activate shaders for next draw call
    glBindProgramPipeline(gl->pipeline);
    
    // Provide vertex input
    glBindVertexArray(gl->vao);
    
    // Bind texture to texture unit
    GLint s_texture = 0; // texture unit that sampler2D will use in GLSL code
    glBindTextureUnit(s_texture, gl->texture);
    
    // Draw 3 vertices as triangle
    glDrawArrays(GL_TRIANGLES, 0, 3);
    
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
