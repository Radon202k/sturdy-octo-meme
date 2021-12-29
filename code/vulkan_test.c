#define VULKAN_USE_VALIDATION_LAYER
#include "win32/win32_core.h"

WIN32_ENTRY()
{
    // Create window
    os.vk.hwnd = win32_make_window(0, 0, 500, 500, "Minecraft clone");
    
    // Init renderer
    vulkan_init(&os.vk);
    
    
    win32_show_window(os.vk.hwnd);
    
    // Main loop
    LARGE_INTEGER lastCounter = win32_get_perfcounter();
    while(os.window_is_open)
    {
        LARGE_INTEGER counter = win32_get_perfcounter();
        f32 elapsedTime = win32_get_elapsed_time(lastCounter, counter);
        lastCounter = counter;
        
        
        
        // Handle OS events
        win32_poll_messages();
        
        vulkan_draw_frame(&os.vk);
    }
    
    // Renderer cleanup
    vulkan_cleanup(&os.vk);
    
    return 0;
}