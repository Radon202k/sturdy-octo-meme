/* date = December 30th 2021 11:12 pm */

#ifndef WIN32_KEYBOARD_H
#define WIN32_KEYBOARD_H

enum keyboard_key
{
    KEY_CONTROL,
    KEY_SPACE,
    
    KEY_A,
    KEY_W,
    KEY_S,
    KEY_D,
    
    KEY_COUNT,
};

typedef struct os_keyboard
{
    os_button buttons[KEY_COUNT];
} os_keyboard;

internal void
win32_handle_keyboardbutton(os_button *button, UINT msg)
{
    b32 down = 0;
    
    if (msg == WM_KEYDOWN)
    {
        down = 1;
    }
    
    button->pressed = down && !button->down;
    button->down = down;
    button->up = !down;
}

#endif //WIN32_KEYBOARD_H
