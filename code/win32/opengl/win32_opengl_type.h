/* date = December 29th 2021 4:35 pm */

#ifndef WIN32_OPENGL_TYPES_H
#define WIN32_OPENGL_TYPES_H

typedef struct opengl_shader
{
    GLuint vShader;
    GLuint fShader;
    GLuint pipeline;
} opengl_shader;

typedef struct opengl_mesh
{
    f32 *vertices;
    u32 vertexCount;
} opengl_mesh;

typedef struct opengl_mesh_indexed
{
    f32 *vertices;
    u32 vertexCount;
    
    u32 *indices;
    u32 indexCount;
} opengl_mesh_indexed;

typedef struct opengl_vertexbuffer
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    size_t vertexSize;
    u32 vertexCount;
    f32 *vertices;
    u32 indexCount;
    u32 *indices;
} opengl_vertexbuffer;

typedef enum renderpass_primitive
{
    renderpass_primitive_triangles,
    renderpass_primitive_lines,
} renderpass_primitive;

typedef struct opengl_renderpass
{
    opengl_vertexbuffer *buffer;
    opengl_shader *shader;
    renderpass_primitive primitiveType;
    GLint textureUnit;
    GLuint textureHandle;
    mat4 view;
    mat4 proj;
} opengl_renderpass;

typedef struct opengl_context
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
    
} opengl_context;

#endif //WIN32_OPENGL_TYPES_H
