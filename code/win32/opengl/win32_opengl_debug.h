/* date = December 29th 2021 4:26 pm */

#ifndef WIN32_OPENGL_DEBUG_H
#define WIN32_OPENGL_DEBUG_H

internal void APIENTRY
opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                      GLsizei length, const GLchar* message, const void* user)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    
    if (severity == GL_DEBUG_SEVERITY_HIGH || severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        if (IsDebuggerPresent())
        {
            assert(!"OpenGL error - check the callstack in debugger");
        }
        
        fatal_error("OpenGL API usage error! Use debugger to examine call stack!");
    }
}

#endif //WIN32_OPENGL_DEBUG_H
