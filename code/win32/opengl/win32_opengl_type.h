/* date = December 29th 2021 4:35 pm */

#ifndef WIN32_OPENGL_TYPES_H
#define WIN32_OPENGL_TYPES_H

typedef struct gl_shader_t
{
    GLuint vShader;
    GLuint fShader;
    GLuint pipeline;
} gl_shader_t;

typedef struct gl_mesh_t
{
    f32 *vertices;
    u32 vertexCount;
    
    u32 *indices;
    u32 indexCount;
} gl_mesh_t;

typedef struct gl_vbuffer_t
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t vertexSize;
    u32 vertexCount;
    f32 *vertices;
    u32 indexCount;
    u32 *indices;
} gl_vbuffer_t;

typedef enum gl_primitive_t
{
    gl_primitive_triangles,
    gl_primitive_lines,
} gl_primitive_t;

typedef struct gl_renderpass_t
{
    gl_vbuffer_t *buffer;
    gl_shader_t *shader;
    gl_primitive_t primitiveType;
    GLint textureUnit;
    GLuint textureHandle;
    u32 lineWidth;
    mat4 view;
    mat4 proj;
} gl_renderpass_t;

typedef struct gl_context_t
{
    HWND hwnd;
    u32 windowWidth;
    u32 windowHeight;
    HDC hdc;
    HGLRC hglrc;
    
    f32 angle;
    
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
    
} gl_context_t;

#endif //WIN32_OPENGL_TYPES_H
