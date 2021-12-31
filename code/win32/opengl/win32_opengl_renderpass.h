/* date = December 31st 2021 4:43 pm */

#ifndef WIN32_OPENGL_RENDERPASS_H
#define WIN32_OPENGL_RENDERPASS_H

internal void
opengl_execute_renderpass(opengl_context *gl, opengl_renderpass renderPass)
{
    f32 aspect = (f32)gl->windowWidth / (f32)gl->windowHeight;
    
    opengl_upload_uniforms(gl, &renderPass.view, &renderPass.proj);
    
    // Activate shaders for next draw call
    glBindProgramPipeline(gl->pipeline);
    
    GLint textureUnit = renderPass.textureUnit;
    GLuint textureHandle = renderPass.textureHandle;
    
    // Provide vertex input
    glBindVertexArray(renderPass.buffer->vao);
    
    // Bind texture to texture unit
    // texture unit that sampler2D will use in GLSL code
    glBindTextureUnit(textureUnit, textureHandle);
    
    // Bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderPass.buffer->ebo);
    
    // Draw 3 vertices as triangle
    glDrawElements(GL_TRIANGLES, renderPass.buffer->indexCount, GL_UNSIGNED_INT, 0);
    
}

#endif //WIN32_OPENGL_RENDERPASS_H
