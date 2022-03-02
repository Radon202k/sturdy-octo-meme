/* date = December 29th 2021 4:39 pm */

#ifndef WIN32_OPENGL_CONTEXT_H
#define WIN32_OPENGL_CONTEXT_H

internal void
opengl_set_pixelformat(gl_context_t *gl, HDC hdc)
{
    PIXELFORMATDESCRIPTOR pixelFormat =
    {
        .nSize = sizeof(pixelFormat),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
    };
    
    // Choose
    int format = ChoosePixelFormat(hdc, &pixelFormat);
    if (!format)
    {
        fatal_error("Cannot choose OpenGL pixel format for dummy window!");
    }
    
    // Describe
    int ok = DescribePixelFormat(hdc, format, sizeof(pixelFormat), &pixelFormat);
    assert(ok && "Failed to describe OpenGL pixel format");
    
    // Set
    if (!SetPixelFormat(hdc, format, &pixelFormat))
    {
        fatal_error("Cannot set OpenGL pixel format for dummy window!");
    }
}

internal void
opengl_prepare_modern_context(gl_context_t *gl)
{
    // Make a dummy window / device context
    HWND hwnd = CreateWindowExW(0, L"STATIC", L"DummyWindow", WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, 0, 0);
    assert(hwnd && "Failed to create dummy window");
    HDC hdc = GetDC(hwnd);
    assert(hdc && "Failed to get device context for dummy window");
    
    // Set pixel format
    opengl_set_pixelformat(gl, hdc);
    
    // Make dummy context
    HGLRC hglrc = wglCreateContext(hdc);
    assert(hglrc && "Failed to create OpenGL context for dummy window");
    int ok = wglMakeCurrent(hdc, hglrc);
    assert(ok && "Failed to make current OpenGL context for dummy window");
    
    // Load necessary extensions
    opengl_load_extensions(gl, hdc);
    
    // Clean up dummy context
    wglMakeCurrent(0, 0);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

internal void
opengl_set_pixelformat_wgl(gl_context_t *gl)
{
    int attrib[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        
        // uncomment for sRGB framebuffer, from WGL_ARB_framebuffer_sRGB extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_framebuffer_sRGB.txt
        //WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
        
        // uncomment for multisampeld framebuffer, from WGL_ARB_multisample extension
        // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_multisample.txt
        //WGL_SAMPLE_BUFFERS_ARB, 1,
        //WGL_SAMPLES_ARB,        4, // 4x MSAA
        
        0,
    };
    
    int format;
    UINT formats;
    if (!gl->wglChoosePixelFormatARB(gl->hdc, attrib, 0, 1, &format, &formats) || formats == 0)
    {
        fatal_error("OpenGL does not support required pixel format!");
    }
    
    PIXELFORMATDESCRIPTOR desc = { .nSize = sizeof(desc) };
    int ok = DescribePixelFormat(gl->hdc, format, sizeof(desc), &desc);
    assert(ok && "Failed to describe OpenGL pixel format");
    
    if (!SetPixelFormat(gl->hdc, format, &desc))
    {
        fatal_error("Cannot set OpenGL selected pixel format!");
    }
}

internal void
opengl_make_modern_context(gl_context_t *gl)
{
    opengl_set_pixelformat_wgl(gl);
    
    int attrib[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        0,
    };
    
    gl->hglrc = gl->wglCreateContextAttribsARB(gl->hdc, 0, attrib);
    if (!gl->hglrc)
    {
        fatal_error("Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
    }
    
    BOOL ok = wglMakeCurrent(gl->hdc, gl->hglrc);
    assert(ok && "Failed to make current OpenGL context");
    
    // load OpenGL functions
#define X(type, name) name = (type)wglGetProcAddress(#name); assert(name);
    GL_FUNCTIONS(X)
#undef X
    
#ifndef NDEBUG
    // enable debug callback
    glDebugMessageCallback(&opengl_debug_callback, 0);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif
}

internal void
opengl_set_global_state(gl_context_t *gl)
{
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable depth testing
    glDepthFunc(GL_GEQUAL);
    glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    glEnable(GL_DEPTH_TEST);
    // Enable culling
    glEnable(GL_CULL_FACE);
}

#endif //WIN32_OPENGL_CONTEXT_H
