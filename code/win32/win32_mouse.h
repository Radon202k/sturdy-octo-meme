/* date = December 21st 2021 1:53 am */

#ifndef WIN32_MOUSE_H
#define WIN32_MOUSE_H

internal void
win32_handle_mousemove(LPARAM lParam)
{
    os.mouse.pos = MAKEPOINTS(lParam); 
}

internal void
win32_handle_mousebutton(os_button *button, UINT msg)
{
    b32 down = 0;
    
    if ((msg == WM_LBUTTONDOWN) || 
        (msg == WM_RBUTTONDOWN) ||
        (msg == WM_MBUTTONDOWN))
    {
        down = 1;
    }
    
    button->pressed = down;
    button->down = down;
    button->up = !down;
}

#endif //WIN32_MOUSE_H
