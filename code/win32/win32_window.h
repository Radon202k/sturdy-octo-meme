/* date = December 21st 2021 0:39 am */

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

internal WNDCLASS
win32_make_wndclass()
{
    WNDCLASSA wc = 
    {
        .lpfnWndProc   = WindowProc,
        .hInstance     = GetModuleHandle(0),
        .lpszClassName = "DefaultWindowClass",
        .style         = CS_OWNDC,
        .lpfnWndProc   = WindowProc,
    };
    
    if (!RegisterClassA(&wc))
    {
        fatal_error("Failed to register window class.");
    }
    
    return wc;
}

internal RECT
win32_get_adjusted_rect(u32 x, u32 y, u32 w, u32 h)
{
    RECT result = 
    {
        .left = x,
        .top = y,
        .right = x + w,
        .bottom = y + h,
    };
    
    AdjustWindowRect(&result, WS_OVERLAPPEDWINDOW, 0);
    
    if (result.left < 0)
    {
        result.right += -result.left;
        result.left = 0;
    }
    
    if (result.top < 0)
    {
        result.bottom += -result.top;
        result.top = 0;
    }
    
    return result;
}

internal HWND
win32_make_window(u32 x, u32 y, u32 w, u32 h, char *title)
{
    WNDCLASS wc = win32_make_wndclass();
    
    RECT rect = win32_get_adjusted_rect(x, y, w, h);
    
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName,
                               title,
                               WS_OVERLAPPEDWINDOW,
                               rect.left, rect.top, 
                               rect.right - rect.left, 
                               rect.bottom - rect.top,
                               0, 0, GetModuleHandle(0), 0);
    
    if (hwnd)
    {
        os.window_is_open = 1;
    }
    else
    {
        fatal_error("Failed to create window.");
    }
    
    return hwnd;
}

internal void
win32_show_window(HWND hwnd)
{
    ShowWindow(hwnd, 1);
}

internal void
win32_hide_window(HWND hwnd)
{
    ShowWindow(hwnd, 0);
}

internal void
win32_close_window(void)
{
    os.window_is_open = 0;
}

#endif //WIN32_WINDOW_H
