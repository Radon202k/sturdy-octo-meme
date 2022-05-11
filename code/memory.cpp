/* date = May 7th 2022 10:23 pm */

#ifndef MEMORY_H
#define MEMORY_H

internal void
allocMemoryPools(void)
{
#ifdef HANE3D_DEBUG
    void *platMemAddress = (void *)terabytes(2);
#else
    void *platMemAddress = 0;
#endif
    u32 platPermSize = (u32)gigabytes(3.2f);
    u32 platTempSize = megabytes(512);
    u32 totalMemSize = platPermSize + platTempSize;
    
    app.permanent = hnInitMandala(platPermSize, hnAllocateMemory(platMemAddress, totalMemSize));
    app.temporary = hnInitMandala(platTempSize, (u8 *)app.permanent.base + platPermSize);
}

#endif //MEMORY_H
