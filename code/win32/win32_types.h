/* date = December 21st 2021 0:38 am */

#ifndef WIN32_TYPES_H
#define WIN32_TYPES_H

typedef struct button_t
{
    // Fires once
    b32 pressed;
    b32 up;
    
    // Fires every frame
    b32 down;
} button_t;

typedef struct mouse_t
{
    v2 pos;
    button_t buttons[3];
} mouse_t;

#endif //WIN32_TYPES_H
