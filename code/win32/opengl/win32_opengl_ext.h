/* date = December 29th 2021 4:36 pm */

#ifndef WIN32_OPENGL_EXTENSIONS_H
#define WIN32_OPENGL_EXTENSIONS_H

typedef void (* PFNGLCLIPCONTROL) (GLenum origin, GLenum depth);

#define GL_FUNCTIONS(X) \
X(PFNGLCREATEBUFFERSPROC,            glCreateBuffers            ) \
X(PFNGLBINDBUFFERPROC,               glBindBuffer               ) \
X(PFNGLNAMEDBUFFERSTORAGEPROC,       glNamedBufferStorage       ) \
X(PFNGLNAMEDBUFFERDATAPROC,          glNamedBufferData          ) \
X(PFNGLBINDVERTEXARRAYPROC,          glBindVertexArray          ) \
X(PFNGLCREATEVERTEXARRAYSPROC,       glCreateVertexArrays       ) \
X(PFNGLVERTEXARRAYATTRIBBINDINGPROC, glVertexArrayAttribBinding ) \
X(PFNGLVERTEXARRAYVERTEXBUFFERPROC,  glVertexArrayVertexBuffer  ) \
X(PFNGLVERTEXARRAYATTRIBFORMATPROC,  glVertexArrayAttribFormat  ) \
X(PFNGLENABLEVERTEXARRAYATTRIBPROC,  glEnableVertexArrayAttrib  ) \
X(PFNGLCREATESHADERPROGRAMVPROC,     glCreateShaderProgramv     ) \
X(PFNGLGETPROGRAMIVPROC,             glGetProgramiv             ) \
X(PFNGLGETPROGRAMINFOLOGPROC,        glGetProgramInfoLog        ) \
X(PFNGLGENPROGRAMPIPELINESPROC,      glGenProgramPipelines      ) \
X(PFNGLUSEPROGRAMSTAGESPROC,         glUseProgramStages         ) \
X(PFNGLBINDPROGRAMPIPELINEPROC,      glBindProgramPipeline      ) \
X(PFNGLPROGRAMUNIFORMMATRIX4FVPROC,  glProgramUniformMatrix4fv  ) \
X(PFNGLBINDTEXTUREUNITPROC,          glBindTextureUnit          ) \
X(PFNGLCREATETEXTURESPROC,           glCreateTextures           ) \
X(PFNGLTEXTUREPARAMETERIPROC,        glTextureParameteri        ) \
X(PFNGLTEXTURESTORAGE2DPROC,         glTextureStorage2D         ) \
X(PFNGLTEXTURESUBIMAGE2DPROC,        glTextureSubImage2D        ) \
X(PFNGLDEBUGMESSAGECALLBACKPROC,     glDebugMessageCallback     ) \
X(PFNGLCLIPCONTROL,                  glClipControl              )

#define X(type, name) static type name;
GL_FUNCTIONS(X)
#undef X

#define STR2(x) #x
#define STR(x) STR2(x)

internal void
opengl_load_extensions(gl_context_t *gl, HDC hdc)
{
    char *version = glGetString(GL_VERSION);
    
    
    // Load function to query extensions
    gl->wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!gl->wglGetExtensionsStringARB)
    {
        fatal_error("OpenGL does not support WGL_ARB_extensions_string extension!");
    }
    
    // Query the extensions for this device
    char *ext = gl->wglGetExtensionsStringARB(hdc);
    assert(ext && "Failed to get OpenGL WGL extension string");
    
    // Load available extensions
    char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
        {
            ext++;
        }
        
        size_t length = ext - start;
        if (strings_are_equal("WGL_ARB_pixel_format", start, length))
        {
            gl->wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (strings_are_equal("WGL_ARB_create_context", start, length))
        {
            gl->wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        else if (strings_are_equal("WGL_EXT_swap_control", start, length))
        {
            gl->wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        }
        
        if (*ext == 0)
        {
            break;
        }
        
        ext++;
        start = ext;
    }
    
    if (!gl->wglChoosePixelFormatARB || !gl->wglCreateContextAttribsARB || !gl->wglSwapIntervalEXT)
    {
        fatal_error("OpenGL does not support required WGL extensions for modern context!");
    }
}

#endif //WIN32_OPENGL_EXTENSIONS_H
