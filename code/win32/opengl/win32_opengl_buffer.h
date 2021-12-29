/* date = December 29th 2021 7:14 pm */

#ifndef WIN32_OPENGL_VERTEXBUFFER_H
#define WIN32_OPENGL_VERTEXBUFFER_H

internal void
opengl_make_buffers(opengl_context *gl)
{
    opengl_vertex data[] =
    {
        { { -0.00f, +0.75f }, { 25.0f, 50.0f }, { 1, 0, 0 } },
        { { +0.75f, -0.50f }, {  0.0f,  0.0f }, { 0, 1, 0 } },
        { { -0.75f, -0.50f }, { 50.0f,  0.0f }, { 0, 0, 1 } },
    };
    
    // Vertex buffer
    glCreateBuffers(1, &gl->vbo);
    glNamedBufferStorage(gl->vbo, sizeof(data), data, 0);
    
    // Vertex array
    glCreateVertexArrays(1, &gl->vao);
    
    // Vao's vbo
    GLint vbuf_index = 0;
    glVertexArrayVertexBuffer(gl->vao, vbuf_index, gl->vbo, 0, sizeof(opengl_vertex));
    
    // Vao's input layout
    GLint a_pos = 0;
    glVertexArrayAttribFormat(gl->vao, a_pos, 2, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, position));
    glVertexArrayAttribBinding(gl->vao, a_pos, vbuf_index);
    glEnableVertexArrayAttrib(gl->vao, a_pos);
    
    GLint a_uv = 1;
    glVertexArrayAttribFormat(gl->vao, a_uv, 2, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, uv));
    glVertexArrayAttribBinding(gl->vao, a_uv, vbuf_index);
    glEnableVertexArrayAttrib(gl->vao, a_uv);
    
    GLint a_color = 2;
    glVertexArrayAttribFormat(gl->vao, a_color, 3, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, color));
    glVertexArrayAttribBinding(gl->vao, a_color, vbuf_index);
    glEnableVertexArrayAttrib(gl->vao, a_color);
}

internal void
opengl_upload_uniform_buffer_data(opengl_context *gl)
{
    gl->angle += 0.002f * 2.0f * 3.1415f / 20.0f; // full rotation in 20 seconds
    gl->angle = fmodf(gl->angle, 2.0f * 3.1415f);
    
    f32 aspect = (f32)gl->windowHeight / gl->windowWidth;
    f32 matrix[] =
    {
        cosf(gl->angle) * aspect, -sinf(gl->angle),
        sinf(gl->angle) * aspect,  cosf(gl->angle),
    };
    
    GLint u_matrix = 0;
    glProgramUniformMatrix2fv(gl->vShader, u_matrix, 1, GL_FALSE, matrix);
}

#endif //WIN32_OPENGL_VERTEXBUFFER_H
