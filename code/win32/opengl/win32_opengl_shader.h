/* date = December 29th 2021 8:00 pm */

#ifndef WIN32_OPENGL_SHADER_H
#define WIN32_OPENGL_SHADER_H

internal void
opengl_make_shaders(opengl_context *gl)
{
    binary_file vShaderFile = read_entire_file("shaders/shader.vert");
    binary_file fShaderFile = read_entire_file("shaders/shader.frag");
    
    gl->vShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vShaderFile.data);
    gl->fShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fShaderFile.data);
    
    GLint linked;
    glGetProgramiv(gl->vShader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char message[1024];
        glGetProgramInfoLog(gl->vShader, sizeof(message), 0, message);
        OutputDebugStringA(message);
        assert(!"Failed to create vertex shader!");
    }
    
    glGetProgramiv(gl->fShader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char message[1024];
        glGetProgramInfoLog(gl->fShader, sizeof(message), 0, message);
        OutputDebugStringA(message);
        assert(!"Failed to create fragment shader!");
    }
    
    glGenProgramPipelines(1, &gl->pipeline);
    glUseProgramStages(gl->pipeline, GL_VERTEX_SHADER_BIT, gl->vShader);
    glUseProgramStages(gl->pipeline, GL_FRAGMENT_SHADER_BIT, gl->fShader);
}

#endif //WIN32_OPENGL_SHADER_H
