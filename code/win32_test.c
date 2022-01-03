#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_RENDERER
#include "win32/win32_core.h"

#include "renderer.h"

#include "perlin_test.c"
#include "cubes_test.c"

typedef enum scene_type_t
{
    scene_type_null,
    scene_type_perlin_test,
    scene_type_cubes_test,
} scene_type_t;

global scene_type_t currentScene = scene_type_perlin_test;
global b32 currentSceneInitialized;

inline void
scene_set_current(memory_pool *pool, scene_type_t newScene)
{
    memset(pool->permBase, 0, pool->permSize);
    memset(pool->tranBase, 0, pool->tranSize);
    
    currentSceneInitialized = 0;
    currentScene = newScene;
}

WIN32_ENTRY()
{
    memory_pool pool = win32_init();
    
    memory_arena platArena;
    arena_init(&platArena, pool.platSize, pool.platBase);
    
    renderer_init(&platArena);
    
    currentScene = scene_type_perlin_test;
    
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
            if (os.keyboard.buttons[KEY_A].pressed)
            {
                scene_set_current(&pool, scene_type_perlin_test);
            }
            else if (os.keyboard.buttons[KEY_D].pressed)
            {
                scene_set_current(&pool, scene_type_cubes_test);
            }
        }
        
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
    }
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}