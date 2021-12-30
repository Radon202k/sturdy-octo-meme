/* date = December 29th 2021 7:45 pm */

#ifndef WIN32_OPENGL_TEXTURE_H
#define WIN32_OPENGL_TEXTURE_H

internal void
opengl_make_texture(GLuint *handle, GLsizei width, GLsizei height, u32 *pixels)
{
    glCreateTextures(GL_TEXTURE_2D, 1, handle);
    glTextureParameteri(*handle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(*handle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTextureParameteri(*handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(*handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTextureStorage2D(*handle, 1, GL_RGBA8, width, height);
    glTextureSubImage2D(*handle, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

internal void
opengl_make_textures(opengl_context *gl)
{
    u32 pixels[] =
    {
        0xffffffff, 0xffAAAAAA,
        0xffAAAAAA, 0xffffffff,
    };
    
    GLsizei width = 2;
    GLsizei height = 2;
    
    opengl_make_texture(&gl->texture, width, height, pixels); 
}

#endif //WIN32_OPENGL_TEXTURE_H
