#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_RENDERER
#include "win32/win32_core.h"

#include "renderer.h"

typedef enum scene_type_t
{
    scene_type_null,
    scene_type_perlin_test,
    scene_type_cubes_test,
} scene_type_t;

global scene_type_t currentScene;
global b32 currentSceneInitialized;

typeless_vector_t mainVertexBuffers;
gl_vbuffer_t *mainLinesVertexBuffer;
gl_vbuffer_t *mainTextVertexBuffer;

gl_renderpass_t *mainRenderPasses;

gl_shader_t textureShader;
gl_shader_t lineShader;

#include "perlin_test.c"
#include "cubes_test.c"

inline void
scene_set_current(memory_pool_t *pool, scene_type_t newScene)
{
    memset(pool->permBase, 0, pool->permSize);
    memset(pool->tempBase, 0, pool->tempSize);
    
    currentSceneInitialized = 0;
    currentScene = newScene;
}

WIN32_ENTRY()
{
    memory_pool_t pool = win32_init();
    
    memory_arena_t platArena;
    arena_init(&platArena, pool.platSize, pool.platBase);
    
    renderer_init(&platArena);
    
    currentScene = scene_type_cubes_test;
    
    // Texture shader
    {
        char vShader[MAX_PATH];
        char fShader[MAX_PATH];
        string_concat(vShader, sizeof(vShader), os.rootPath, "\\..\\code\\shaders\\main.vert");
        string_concat(fShader, sizeof(fShader), os.rootPath, "\\..\\code\\shaders\\main.frag");
        textureShader = opengl_make_shader(&os.gl, vShader, fShader);
    }
    
    // Lines shader
    {
        char vShader[MAX_PATH];
        char fShader[MAX_PATH];
        string_concat(vShader, sizeof(vShader), os.rootPath, "\\..\\code\\shaders\\lines.vert");
        string_concat(fShader, sizeof(fShader), os.rootPath, "\\..\\code\\shaders\\lines.frag");
        lineShader = opengl_make_shader(&os.gl, vShader, fShader);
    }
    
    // Main vertex buffers
    {
        // Make vertex buffer array
        mainVertexBuffers = make_typeless_vector(2, sizeof(gl_vbuffer_t));
        
        mainLinesVertexBuffer = &((gl_vbuffer_t *)mainVertexBuffers.data)[0];
        mainTextVertexBuffer = &((gl_vbuffer_t *)mainVertexBuffers.data)[1];
        
        *mainLinesVertexBuffer = opengl_make_vbuffer(&platArena, sizeof(f32)*3+sizeof(u32), megabytes(2));
        opengl_vbuffer_set_inputlayout(mainLinesVertexBuffer, 0, GL_FLOAT, 3, 0);
        opengl_vbuffer_set_inputlayout(mainLinesVertexBuffer, 1, GL_UNSIGNED_BYTE, 4, sizeof(f32)*3);
        
        //render_line(mainLinesVertexBuffer, V3(0,0,-1), V3(200,200,-1), V3(0,1,0));
        
        //opengl_upload_vbuffer_data(mainLinesVertexBuffer);
        
        *mainTextVertexBuffer = opengl_make_vbuffer(&platArena, sizeof(f32)*8, megabytes(2));
        opengl_vbuffer_set_default_inputlayout(mainTextVertexBuffer);
    }
    
    // Main render passes
    {
        mat4 noView = mat4_identity(1.0f);
        mat4 orthographicProj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
        mainRenderPasses = push_array(&platArena, 2, gl_renderpass_t, 4);
        
        
        mainRenderPasses[0] = opengl_make_renderpass(mainLinesVertexBuffer, gl_primitive_lines, 0, 0, 10,
                                                     noView, orthographicProj, &lineShader);
        
        mainRenderPasses[1] = opengl_make_renderpass(mainTextVertexBuffer, gl_primitive_triangles,
                                                     0, globalFontTexture, 0, noView, orthographicProj, &textureShader);
        
        
    }
    
    // Main loop
    LARGE_INTEGER lastCounter = win32_get_perfcounter();
    while(os.window_is_open)
    {
        LARGE_INTEGER counter = win32_get_perfcounter();
        f32 elapsedTime = win32_get_elapsed_time(lastCounter, counter);
        lastCounter = counter;
        
        // Handle OS events
        win32_poll_messages();
        
        if (os.keyboard.buttons[KEY_CONTROL].down)
        {
            if (os.keyboard.buttons[KEY_1].pressed)
            {
                scene_set_current(&pool, scene_type_perlin_test);
            }
            else if (os.keyboard.buttons[KEY_2].pressed)
            {
                scene_set_current(&pool, scene_type_cubes_test);
            }
        }
        
        // Prepare frame for drawing
        opengl_begin_frame(&os.gl);
        
        
        switch (currentScene)
        {
            case scene_type_perlin_test:
            {
                if (!currentSceneInitialized)
                {
                    perlin_scene_init(&pool);
                    currentSceneInitialized = 1;
                }
                
                perlin_scene_update(&pool, elapsedTime);
            } break;
            
            case scene_type_cubes_test:
            {
                if (!currentSceneInitialized)
                {
                    cubes_scene_init(&pool);
                    currentSceneInitialized = 1;
                }
                
                cubes_scene_update(&pool, elapsedTime);
            } break;
            
            default:
            {
            } break;
        }
        
        
        // Main render passes
        {
            // Begin temporary memory
            temp_memory_t mem = begin_temp_memory(&platArena);
            
            // Debug text
            {
                mainTextVertexBuffer->vertexCount = 0;
                mainTextVertexBuffer->indexCount = 0;
                
                char fpsBuffer[256] = {0};
                win32_make_label_f32(fpsBuffer, sizeof(fpsBuffer), "Fps", 1.0f / elapsedTime);
                
                stbtt_print(&platArena, mainTextVertexBuffer, 0, 30, fpsBuffer);
                
                opengl_upload_vbuffer_data(mainTextVertexBuffer);
            }
            
            // End temporary memory
            end_temp_memory(mem);
            
            // Update render passes
            mainRenderPasses[0].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
            mainRenderPasses[1].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
            
        }
        
        opengl_execute_renderpasses(&os.gl, mainRenderPasses, 2);
        
        opengl_end_frame(&os.gl);
    }
    
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}