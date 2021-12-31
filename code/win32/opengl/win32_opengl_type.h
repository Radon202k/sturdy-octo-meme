/* date = December 29th 2021 4:35 pm */

#ifndef WIN32_OPENGL_TYPES_H
#define WIN32_OPENGL_TYPES_H

typedef struct opengl_vertex
{
    f32 position[3];
    f32 uv[2];
    f32 color[3];
    GLuint texture;
} opengl_vertex;

typedef struct opengl_mesh
{
    opengl_vertex *vertices;
    u32 vertexCount;
} opengl_mesh;

typedef struct opengl_mesh_indexed
{
    opengl_vertex *vertices;
    u32 vertexCount;
    
    u32 *indices;
    u32 indexCount;
} opengl_mesh_indexed;

typedef struct opengl_vertexbuffer
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    u32 vertexCount;
    u32 indexCount;
} opengl_vertexbuffer;

typedef struct opengl_context
{
    HWND hwnd;
    u32 windowWidth;
    u32 windowHeight;
    HDC hdc;
    HGLRC hglrc;
    
    GLuint texture;
    GLuint vShader;
    GLuint fShader;
    GLuint pipeline;
    f32 angle;
    
    typeless_vector vertexBuffers;
    
    PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
    
} opengl_context;

#endif //WIN32_OPENGL_TYPES_H
