/* date = May 7th 2022 9:58 pm */

#ifndef VOXEL_H
#define VOXEL_H

#define CHUNK_SIZE v3{16,16,16}

struct world_pos
{
    s32 absX;
    s32 absY;
    s32 absZ;
    
    v3 offset;
};

struct chunk_pos
{
    s32 chunkX;
    s32 chunkY;
    s32 chunkZ;
    
    s32 relVoxelX;
    s32 relVoxelY;
    s32 relVoxelZ;
};

struct chunk
{
    b32 active;
    
    s32 chunkX;
    s32 chunkY;
    s32 chunkZ;
    
    u32 *voxels;
    
    chunk *freePrev;
    chunk *freeNext;
    
    chunk *nextInHash;
    chunk *prevInHash;
    
    b32 cameFromFreeList;
    
    hnGpuBuffer *vb;
    hnGpuBuffer *ib;
    
    f32 maxHeights[(s32)CHUNK_SIZE.z][(s32)CHUNK_SIZE.x];
};

struct voxel_map
{
    hnMandala *permanent;
    
    s32 chunkShift;
    s32 chunkMask;
    s32 chunkDim;
    
    chunk *hash[CHUNK_HASH_TABLE_SIZE];
    chunk *freeFirst;
    
    v3 viewDist; // Integer chunk units (must have all dimensions equal)
    v3 oldCenter; // Integer chunk coordinates
    v3 currentCenter; // Integer chunk coordinates
};

inline u32
getChunkHashIndex(voxel_map *map, s32 x, s32 y, s32 z)
{
    assert(arrayCount(map->hash) % 2 == 0);
    // TODO: Better hash function !!
    u32 hashIndex = (u32)(x * 19 + y * 7 + z * 3);
    u32 hashSlot = hashIndex % (arrayCount(map->hash) - 1);
    assert(hashSlot < arrayCount(map->hash));
    return hashSlot;
}

inline s32
getVoxelIndex(s32 x, s32 y, s32 z)
{
    s32 result = (s32)(z*CHUNK_SIZE.x*CHUNK_SIZE.y + y*CHUNK_SIZE.x + x);
    return result;
}

#endif //VOXEL_H
