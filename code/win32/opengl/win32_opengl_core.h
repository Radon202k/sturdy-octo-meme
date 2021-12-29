/* date = December 29th 2021 4:19 pm */

#ifndef WIN32_OPENGL_CORE_H
#define WIN32_OPENGL_CORE_H

#include "win32_opengl_types.h"
#include "win32_opengl_debug.h"
#include "win32_opengl_extensions.h"
#include "win32_opengl_context.h"

internal void
opengl_init(opengl_context *gl, u32 x, u32 y, u32 w, u32 h, char *title)
{
    opengl_prepare_modern_context(gl);
    
    gl->hwnd = win32_make_window(x, y, w, h, title);
    gl->hdc = GetDC(gl->hwnd);
    
    opengl_make_modern_context(gl);
    
    win32_show_window(gl->hwnd);
}

internal void
opengl_draw_frame(opengl_context *gl)
{
    
}

internal void
opengl_cleanup(opengl_context *gl)
{
    
}

#endif //WIN32_OPENGL_CORE_H
