/* date = December 30th 2021 3:46 pm */

#ifndef WIN32_MEMORY_H
#define WIN32_MEMORY_H

internal void
win32_allocate_memory_pool(void *base, u8 **address, size_t poolSize)
{
    *address = VirtualAlloc(base, poolSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

#endif //WIN32_MEMORY_H
