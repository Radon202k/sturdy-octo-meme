#ifndef WIN32_EVENTS_H
#define WIN32_EVENTS_H


internal void
win32_clear_mouse_buttons(void)
{
    for (u32 i = 0;
         i < 3;
         ++i)
    {
        os.mouse.buttons[i].pressed = 0;
    }
}

internal void
win32_poll_messages()
{
    MSG msg;
    while(PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    
    win32_clear_mouse_buttons();
}

LRESULT CALLBACK 
WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    
    switch(msg)
    {
        case WM_SIZE:
        {
            // Avoid unecessary swapchain recreation on app startup
            if (os.initialized)
            {
                os.vk.swapChain.frameBufferResized = 1;
            }
            else
            {
                os.initialized = 1; // Keep track of first WM_SIZE call
            }
        } break;
        
        case WM_CLOSE:
        case WM_DESTROY:
        {
            win32_close_window();
        } break;
        
        case WM_MOUSEMOVE:
        {
            win32_handle_mousemove(lParam);
        } break;
        
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        {
            win32_handle_mousebutton(&os.mouse.buttons[0], msg);
        } break;
        
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        {
            win32_handle_mousebutton(&os.mouse.buttons[1], msg);
        } break;
        
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        {
            win32_handle_mousebutton(&os.mouse.buttons[2], msg);
        } break;
        
        default:
        {
            result = DefWindowProc(hwnd, msg, wParam, lParam);
        } break;
    }
    
    return result;
}

#endif //WIN32_EVENTS_H
