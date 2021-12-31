/* date = December 30th 2021 11:12 pm */

#ifndef WIN32_KEYBOARD_H
#define WIN32_KEYBOARD_H

#define KEY_CONTROL 0
#define KEY_SPACE 1

internal void
win32_handle_keyboardbutton(os_button *button, UINT msg)
{
    b32 down = 0;
    
    if (msg == WM_KEYDOWN)
    {
        down = 1;
    }
    
    button->pressed = down;
    button->down = down;
    button->up = !down;
}

#endif //WIN32_KEYBOARD_H
