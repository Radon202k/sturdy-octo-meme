/* date = December 29th 2021 8:00 pm */

#ifndef WIN32_OPENGL_SHADER_H
#define WIN32_OPENGL_SHADER_H

internal gl_shader_t
opengl_make_shader(gl_context_t *gl, char *vertexFile, char *fragmentFile)
{
    gl_shader_t result = {0};
    
    binary_file vShaderFile = read_entire_file(vertexFile);
    binary_file fShaderFile = read_entire_file(fragmentFile);
    
    result.vShader = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vShaderFile.data);
    result.fShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fShaderFile.data);
    
    GLint linked;
    glGetProgramiv(result.vShader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char message[1024];
        glGetProgramInfoLog(result.vShader, sizeof(message), 0, message);
        OutputDebugStringA(message);
        assert(!"Failed to create vertex shader!");
    }
    
    glGetProgramiv(result.fShader, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char message[1024];
        glGetProgramInfoLog(result.fShader, sizeof(message), 0, message);
        OutputDebugStringA(message);
        assert(!"Failed to create fragment shader!");
    }
    
    glGenProgramPipelines(1, &result.pipeline);
    glUseProgramStages(result.pipeline, GL_VERTEX_SHADER_BIT, result.vShader);
    glUseProgramStages(result.pipeline, GL_FRAGMENT_SHADER_BIT, result.fShader);
    
    return result;
}

#endif //WIN32_OPENGL_SHADER_H
