#ifndef WIN32_CORE_H
#define WIN32_CORE_H

#include "../core.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32_utils.h"
#include "win32_window.h"
#include "win32_types.h"
#include "win32_memory.h"

#ifdef VULKAN_RENDERER
#include "vulkan/win32_vulkan.h"
#endif

#ifdef OPENGL_RENDERER
#include "opengl/win32_opengl.h"
#endif

#include "win32_keyboard.h"

typedef struct system_context_t
{
    b32 initialized;
    b32 window_is_open;
    
#ifdef VULKAN_RENDERER
    vulkan_context vk;
#endif
    
#ifdef OPENGL_RENDERER
    gl_context_t gl;
#endif
    
    mouse_t mouse;
    os_keyboard keyboard;
    
    f32 perfFrequency;
    
    u8 *memoryPool;
    
    char rootPath[MAX_PATH];
    
} system_context_t;

global system_context_t os;

#include "win32_mouse.h"
#include "win32_events.h"
#include "win32_time.h"

internal memory_pool_t
win32_init(void)
{
    // Time
    LARGE_INTEGER perfFrequency;
    QueryPerformanceFrequency(&perfFrequency);
    os.perfFrequency = (f32)perfFrequency.QuadPart;
    
    // Memory
    size_t platSize = megabytes(2);
    u8 *platBase = 0;
    win32_allocate_memory_pool((void *)terabytes(1), &platBase, platSize);
    
    size_t permSize = megabytes(512);
    size_t tranSize = megabytes(256);
    size_t poolSize = permSize + tranSize;
    win32_allocate_memory_pool((void *)terabytes(2), &os.memoryPool, poolSize);
    
    memory_pool_t result = {0};
    memory_pool_init(&result, platBase, platSize, os.memoryPool, permSize, tranSize);
    
    win32_get_root_path(os.rootPath);
    
    return result;
}

#endif //WIN32_CORE_H