/* date = December 29th 2021 7:45 pm */

#ifndef WIN32_OPENGL_TEXTURE_H
#define WIN32_OPENGL_TEXTURE_H

internal void
opengl_make_texture(GLuint *handle, GLsizei width, GLsizei height, u32 *pixels, GLenum format, GLenum filter)
{
    glCreateTextures(GL_TEXTURE_2D, 1, handle);
    glTextureParameteri(*handle, GL_TEXTURE_MIN_FILTER, filter);
    glTextureParameteri(*handle, GL_TEXTURE_MAG_FILTER, filter);
    glTextureParameteri(*handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(*handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    GLenum internalFormat = GL_RGBA8;
    if (format == GL_RGBA)
    {
        internalFormat = GL_RGBA8;
    }
    else if (format == GL_RED)
    {
        internalFormat = GL_R8;
    }
    
    glTextureStorage2D(*handle, 1, internalFormat, width, height);
    glTextureSubImage2D(*handle, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, pixels);
}

#endif //WIN32_OPENGL_TEXTURE_H
