/* date = December 29th 2021 7:14 pm */

#ifndef WIN32_OPENGL_VERTEXBUFFER_H
#define WIN32_OPENGL_VERTEXBUFFER_H

internal void
opengl_make_buffers(opengl_context *gl, memory_arena *arena)
{
    u32 vertexCount = 0;
    u32 indexCount = 0;
    
    u32 maxVertexCount = 500000;
    u32 maxIndexCount = 500000;
    
    opengl_vertex *vertices = push_array(arena, maxVertexCount, opengl_vertex, 4);
    u32 *indices = push_array(arena, maxIndexCount, u32, 4);
    
    for (u32 k = 0;
         k < 16;
         ++k)
    {
        for (u32 j = 0;
             j < 16;
             ++j)
        {
            for (u32 i = 0;
                 i < 16;
                 ++i)
            {
                f32 scale = 0.2f;
                v3 size = {scale, scale, scale};
                v3 offset = {(f32)i * size.x, (f32)j * size.y, (f32)k * size.z};
                opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, vertexCount);
                
                memcpy(vertices + vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                memcpy(indices + indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                
                // Register the amount of vertices added
                vertexCount += cube.vertexCount;
                indexCount += cube.indexCount;
            }
        }
    }
    
    // Save vertex count to know how many triangles to draw
    gl->vboVertexCount = vertexCount;
    gl->vboIndexCount = indexCount;
    
    // Vertex buffer
    glCreateBuffers(1, &gl->vbo);
    glNamedBufferStorage(gl->vbo, sizeof(opengl_vertex) * vertexCount, vertices, 0);
    
    // Index buffer
    glCreateBuffers(1, &gl->ebo);
    // glNamedBufferStorage(gl->ebo, sizeof(u32) * indexCount, indices, 0);
    glNamedBufferData(gl->ebo, sizeof(u32) * indexCount, indices, GL_STATIC_DRAW);
    
    // Vertex array
    glCreateVertexArrays(1, &gl->vao);
    
    // Vao's vbo
    GLint vbuf_index = 0;
    glVertexArrayVertexBuffer(gl->vao, vbuf_index, gl->vbo, 0, sizeof(opengl_vertex));
    
    // Vao's input layout
    GLint a_pos = 0;
    glVertexArrayAttribFormat(gl->vao, a_pos, 3, GL_FLOAT, GL_FALSE, offsetof(opengl_vertex, position));
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
opengl_upload_uniforms(opengl_context *gl, os_mouse *mouse)
{
    gl->angle += 0.02f * 2.0f * 3.1415f / 20.0f; // full rotation in 20 seconds
    gl->angle = fmodf(gl->angle, 2.0f * 3.1415f);
    
    f32 aspect = (f32)gl->windowHeight / gl->windowWidth;
    
    mat4_inv proj = mat4_perspective(aspect, 0.8f, 0.1f, 100.0f);
    
    f32 dolly = 2;
    
    mat4 cameraM = build_camera_object_matrix(V3(0,0,0), 0.02f * mouse->pos.x, -0.02f * mouse->pos.y, dolly);
    
    mat4_inv view = camera_transform(mat4_get_column(cameraM, 0),
                                     mat4_get_column(cameraM, 1),
                                     mat4_get_column(cameraM, 2),
                                     mat4_get_column(cameraM, 3));
    
    mat4 projViewMat = mat4_transpose(mat4_mul(proj.forward, view.forward));
    
    GLint projview = 0;
    // glProgramUniformMatrix4fv(gl->vShader, projview, 1, GL_FALSE, (f32 *)identity.E);
    glProgramUniformMatrix4fv(gl->vShader, projview, 1, GL_FALSE, (f32 *)projViewMat.E);
}

#endif //WIN32_OPENGL_VERTEXBUFFER_H
