/* date = May 7th 2022 9:58 pm */

#ifndef VOXEL_H
#define VOXEL_H

#define CHUNK_SIZE v3{16,16,16}

struct chunk
{
    b32 active;
    
    v3 p; // Integer coordinates
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
    u32 hashIndex = (u32)(x * 17 + y * 39 + z * 121);
    return hashIndex % arrayCount(map->hash);
}

inline s32
getVoxelIndex(s32 x, s32 y, s32 z)
{
    s32 result = (s32)(z*CHUNK_SIZE.x*CHUNK_SIZE.y + y*CHUNK_SIZE.x + x);
    return result;
}

#endif //VOXEL_H
