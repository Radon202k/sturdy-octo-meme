/* date = December 31st 2021 4:43 pm */

#ifndef WIN32_OPENGL_RENDERPASS_H
#define WIN32_OPENGL_RENDERPASS_H

internal gl_renderpass_t 
opengl_make_renderpass(gl_vbuffer_t *b, gl_primitive_t primitiveType, 
                       GLint texUnit, GLuint texHandle, u32 lineWidth,
                       mat4 view, mat4 proj, gl_shader_t *shader)
{
    gl_renderpass_t renderPass = 
    {
        .shader = shader,
        .buffer = b,
        .primitiveType = primitiveType,
        .textureUnit = texUnit,
        .textureHandle = texHandle,
        .lineWidth = lineWidth,
        .view = view,
        .proj = proj,
    };
    
    return renderPass;
}

internal void
opengl_execute_renderpass(gl_context_t *gl, gl_renderpass_t renderPass)
{
    f32 aspect = (f32)gl->windowWidth / (f32)gl->windowHeight;
    
    GLint textureUnit = renderPass.textureUnit;
    GLuint textureHandle = renderPass.textureHandle;
    
    // Provide vertex input
    glBindVertexArray(renderPass.buffer->vao);
    
    // Activate shaders for next draw call
    glBindProgramPipeline(renderPass.shader->pipeline);
    
    opengl_upload_uniforms(gl, renderPass.shader, &renderPass.view, &renderPass.proj);
    
    // Bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderPass.buffer->ebo);
    
    switch(renderPass.primitiveType)
    {
        case gl_primitive_triangles:
        {
            // Bind texture to texture unit
            // texture unit that sampler2D will use in GLSL code
            glBindTextureUnit(textureUnit, textureHandle);
            
            glDrawElements(GL_TRIANGLES, renderPass.buffer->indexCount, GL_UNSIGNED_INT, 0);
        } break;
        
        case gl_primitive_lines:
        {
            glLineWidth((f32)renderPass.lineWidth);
            
            glDrawElements(GL_LINES, renderPass.buffer->indexCount, GL_UNSIGNED_INT, 0);
        } break;
        
        // invalidDefaultCase;
    }
}

internal void
opengl_execute_renderpasses(gl_context_t *gl, gl_renderpass_t renderPasses[], u32 count)
{
    for (u32 i = 0;
         i < count;
         ++i)
    {
        opengl_execute_renderpass(gl, renderPasses[i]);
    }
}

#endif //WIN32_OPENGL_RENDERPASS_H
