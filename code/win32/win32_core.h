#ifndef WIN32_CORE_H
#define WIN32_CORE_H

#include "../core.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "win32_utils.h"
#include "win32_window.h"
#include "win32_types.h"

#ifdef VULKAN_RENDERER
#include "vulkan/win32_vulkan.h"
#endif

#ifdef OPENGL_RENDERER
#include "opengl/win32_opengl.h"
#endif
typedef struct os_globals
{
    b32 initialized;
    b32 window_is_open;
    
#ifdef VULKAN_RENDERER
    vulkan_context vk;
#endif
    
#ifdef OPENGL_RENDERER
    opengl_context gl;
#endif
    
    os_mouse mouse;
    
    f32 perfFrequency;
    
} os_globals;

global os_globals os;

#include "win32_mouse.h"
#include "win32_events.h"
#include "win32_time.h"

#endif //WIN32_CORE_H