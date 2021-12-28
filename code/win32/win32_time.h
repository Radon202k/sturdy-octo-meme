/* date = December 28th 2021 3:24 pm */

#ifndef WIN32_TIME_H
#define WIN32_TIME_H

internal LARGE_INTEGER
win32_get_perfcounter(void)
{
    LARGE_INTEGER perfCounter;
    QueryPerformanceCounter(&perfCounter);
    return perfCounter;
}

internal f32
win32_get_elapsed_time(LARGE_INTEGER start, LARGE_INTEGER end)
{
    f32 result = (f32)(end.QuadPart - start.QuadPart) / os.perfFrequency;
    return result;
}

#endif //WIN32_TIME_H
