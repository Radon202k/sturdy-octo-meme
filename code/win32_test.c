#define OPENGL_RENDERER
#include "win32/win32_core.h"

WIN32_ENTRY()
{
    opengl_init(&os.gl, 0, 0, 500, 500, "Minecraft clone");
    
    os.window_is_open = 1;
    
    LARGE_INTEGER perfFrequency;
    QueryPerformanceFrequency(&perfFrequency);
    os.perfFrequency = (f32)perfFrequency.QuadPart;
    
    // Main loop
    LARGE_INTEGER lastCounter = win32_get_perfcounter();
    while(os.window_is_open)
    {
        LARGE_INTEGER counter = win32_get_perfcounter();
        f32 elapsedTime = win32_get_elapsed_time(lastCounter, counter);
        lastCounter = counter;
        
        // Handle OS events
        win32_poll_messages();
        
        opengl_draw_frame(&os.gl, &os.mouse);
    }
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}