/* date = December 21st 2021 0:41 am */

#ifndef WIN32_UTILS_H
#define WIN32_UTILS_H

#define WIN32_ENTRY() int WINAPI WinMain(HINSTANCE instance, \
HINSTANCE prevInstance, \
PSTR cmdLine, \
int cmdShow)

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

internal void
show_message(char *msg)
{
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}

internal void
fatal_error(char *msg)
{
    assert(!msg);
    exit(0);
}

typedef struct window_size
{
    u32 width;
    u32 height;
} window_size;

internal window_size
win32_get_window_size(HWND hwnd)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    window_size result =
    {
        .width = clientRect.right - clientRect.left,
        .height = clientRect.bottom - clientRect.top,
    };
    
    return result;
}

#endif //WIN32_UTILS_H
