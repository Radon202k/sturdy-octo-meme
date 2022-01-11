/* date = December 29th 2021 7:14 pm */

#ifndef WIN32_OPENGL_VERTEXBUFFER_H
#define WIN32_OPENGL_VERTEXBUFFER_H

internal void
opengl_upload_vertexbuffer_data_immutable(opengl_vertexbuffer *b)
{
    glNamedBufferStorage(b->vbo, b->vertexSize * b->vertexCount, b->vertices, 0);
    glNamedBufferData(b->ebo, sizeof(u32) * b->indexCount, b->indices, GL_STATIC_DRAW);
}

internal void
opengl_upload_vertexbuffer_data(opengl_vertexbuffer *b)
{
    glNamedBufferData(b->vbo, b->vertexSize * b->vertexCount, b->vertices, GL_DYNAMIC_DRAW);
    glNamedBufferData(b->ebo, sizeof(u32) * b->indexCount, b->indices, GL_DYNAMIC_DRAW);
}

internal opengl_vertexbuffer
opengl_make_vertexbuffer(memory_arena *arena, GLsizei vertexSize, u32 maxVertexCount, u32 maxIndexCount)
{
    opengl_vertexbuffer result = 
    {
        .vao = (GLuint)-1,
        .vbo = (GLuint)-1,
        .ebo = (GLuint)-1,
        .vertexSize = vertexSize,
        .vertexCount = 0,
        .vertices = push_size(arena, maxVertexCount*vertexSize, 4),
        .indexCount = 0,
        .indices = push_array(arena, maxIndexCount, u32, 4),
    };
    
    // Create objects/handles
    glCreateBuffers(1, &result.vbo);
    glCreateBuffers(1, &result.ebo);
    glCreateVertexArrays(1, &result.vao);
    
    // Vao's vbo
    GLint vbuf_index = 0;
    glVertexArrayVertexBuffer(result.vao, vbuf_index, result.vbo, 0, vertexSize);
    
    return result;
}

internal void
opengl_vertexbuffer_set_inputlayout(opengl_vertexbuffer *vertexBuffer, GLint index, GLenum type, 
                                    GLint count, GLuint offset)
{
    glVertexArrayAttribFormat(vertexBuffer->vao, index, count, type, GL_FALSE, offset);
    glVertexArrayAttribBinding(vertexBuffer->vao, index, 0);
    glEnableVertexArrayAttrib(vertexBuffer->vao, index);
}

internal void
opengl_vertexbuffer_set_default_inputlayout(opengl_vertexbuffer *vertexBuffer)
{
    // Vao's input layout
    opengl_vertexbuffer_set_inputlayout(vertexBuffer, 0, GL_FLOAT, 3, 0);
    opengl_vertexbuffer_set_inputlayout(vertexBuffer, 1, GL_FLOAT, 2, sizeof(f32)*3);
    opengl_vertexbuffer_set_inputlayout(vertexBuffer, 2, GL_FLOAT, 3, sizeof(f32)*5);
}

internal void
opengl_upload_uniforms(opengl_context *gl, opengl_shader *shader, mat4 *view, mat4 *proj)
{
    gl->angle += 0.02f * 2.0f * 3.1415f / 20.0f; // full rotation in 20 seconds
    gl->angle = fmodf(gl->angle, 2.0f * 3.1415f);
    
    
    mat4 projViewMat = mat4_transpose(mat4_mul(*proj, *view));
    
    GLint projview = 0;
    // glProgramUniformMatrix4fv(gl->vShader, projview, 1, GL_FALSE, (f32 *)identity.E);
    glProgramUniformMatrix4fv(shader->vShader, projview, 1, GL_FALSE, (f32 *)projViewMat.E);
}

#endif //WIN32_OPENGL_VERTEXBUFFER_H
