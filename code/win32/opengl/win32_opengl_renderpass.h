/* date = December 31st 2021 4:43 pm */

#ifndef WIN32_OPENGL_RENDERPASS_H
#define WIN32_OPENGL_RENDERPASS_H

internal void
opengl_renderpass(opengl_vertexbuffer *b, GLint textureUnit, GLuint textureHandle)
{
    // Provide vertex input
    glBindVertexArray(b->vao);
    
    // Bind texture to texture unit
    // texture unit that sampler2D will use in GLSL code
    glBindTextureUnit(textureUnit, textureHandle);
    
    // Bind index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b->ebo);
    
    // Draw 3 vertices as triangle
    glDrawElements(GL_TRIANGLES, b->indexCount, GL_UNSIGNED_INT, 0);
    
}

#endif //WIN32_OPENGL_RENDERPASS_H
