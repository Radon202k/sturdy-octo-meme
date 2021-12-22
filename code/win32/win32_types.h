/* date = December 21st 2021 0:38 am */

#ifndef WIN32_TYPES_H
#define WIN32_TYPES_H

typedef struct os_button
{
    // Fires once
    b32 pressed;
    b32 up;
    
    // Fires every frame
    b32 down;
} os_button;

typedef struct os_mouse
{
    POINTS pos;
    os_button buttons[3];
} os_mouse;

typedef struct os_globals
{
    b32 initialized;
    b32 window_is_open;
    vulkan_context vk;
    os_mouse mouse;
} os_globals;

#endif //WIN32_TYPES_H
