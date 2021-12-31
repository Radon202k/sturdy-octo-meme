/* date = December 29th 2021 7:14 pm */

#ifndef WIN32_OPENGL_VERTEXBUFFER_H
#define WIN32_OPENGL_VERTEXBUFFER_H

internal void
opengl_upload_vertexbuffer_data_immutable(opengl_vertexbuffer *b, 
                                          u32 vertexCount, opengl_vertex *vertices, 
                                          u32 indexCount, u32 *indices)
{
    b->vertexCount = vertexCount;
    b->indexCount = indexCount;
    
    glNamedBufferStorage(b->vbo, sizeof(opengl_vertex) * vertexCount, vertices, 0);
    glNamedBufferData(b->ebo, sizeof(u32) * indexCount, indices, GL_STATIC_DRAW);
}

internal void
opengl_upload_vertexbuffer_data(opengl_vertexbuffer *b, 
                                u32 vertexCount, opengl_vertex *vertices, 
                                u32 indexCount, u32 *indices)
{
    b->vertexCount = vertexCount;
    b->indexCount = indexCount;
    
    glNamedBufferData(b->vbo, sizeof(opengl_vertex) * vertexCount, vertices, GL_DYNAMIC_DRAW);
    glNamedBufferData(b->ebo, sizeof(u32) * indexCount, indices, GL_DYNAMIC_DRAW);
}


internal opengl_vertexbuffer
opengl_make_vertexbuffer(void)
{
    opengl_vertexbuffer result = 
    {
        .vao = (GLuint)-1,
        .vbo = (GLuint)-1,
        .ebo = (GLuint)-1,
        .vertexCount = 0,
        .indexCount = 0,
        
    };
    
    // Vertex buffer
    glCreateBuffers(1, &result.vbo);
    
    // Index buffer
    glCreateBuffers(1, &result.ebo);
    
    // Vertex array
    glCreateVertexArrays(1, &result.vao);
    
    // Vao's vbo
    GLint vbuf_index = 0;
    glVertexArrayVertexBuffer(result.vao, vbuf_index, result.vbo, 0, sizeof(opengl_vertex));
    
    // Vao's input layout
    GLint a_pos = 0;
    glVertexArrayAttribFormat(result.vao, a_pos, 3, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, position));
    glVertexArrayAttribBinding(result.vao, a_pos, vbuf_index);
    glEnableVertexArrayAttrib(result.vao, a_pos);
    
    GLint a_uv = 1;
    glVertexArrayAttribFormat(result.vao, a_uv, 2, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, uv));
    glVertexArrayAttribBinding(result.vao, a_uv, vbuf_index);
    glEnableVertexArrayAttrib(result.vao, a_uv);
    
    GLint a_color = 2;
    glVertexArrayAttribFormat(result.vao, a_color, 3, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, color));
    glVertexArrayAttribBinding(result.vao, a_color, vbuf_index);
    glEnableVertexArrayAttrib(result.vao, a_color);
    
    return result;
}

internal void
opengl_upload_uniforms(opengl_context *gl, mat4 *view)
{
    gl->angle += 0.02f * 2.0f * 3.1415f / 20.0f; // full rotation in 20 seconds
    gl->angle = fmodf(gl->angle, 2.0f * 3.1415f);
    
    f32 aspect = (f32)gl->windowWidth / (f32)gl->windowHeight;
    
    mat4_inv proj = mat4_perspective(aspect, 0.8f, 0.1f, 1000.0f);
    
    mat4 projViewMat = mat4_transpose(mat4_mul(proj.forward, *view));
    
    GLint projview = 0;
    // glProgramUniformMatrix4fv(gl->vShader, projview, 1, GL_FALSE, (f32 *)identity.E);
    glProgramUniformMatrix4fv(gl->vShader, projview, 1, GL_FALSE, (f32 *)projViewMat.E);
}

#endif //WIN32_OPENGL_VERTEXBUFFER_H
