/* date = May 7th 2022 9:58 pm */

#ifndef VOXEL_H
#define VOXEL_H

#define CHUNK_SIZE v3{16,1,16}

struct chunk
{
    v3 p; // Integer coordinates
    u32 *voxels;
    chunk *freeNext;
    chunk *nextInHash;
    
    render_buffer *gpuBuffer;
};

struct voxel_map
{
    hnMandala *permanent;
    
    chunk *hash[128];
    chunk *freeFirst;
    
    v3 viewDist; // Integer chunk units
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

inline s32
getVoxel(chunk *c, s32 x, s32 y, s32 z)
{
    s32 index = getVoxelIndex(x, y, z);
    s32 result = c->voxels[index];
    return result;
}

inline void
setVoxel(chunk *c, s32 x, s32 y, s32 z, u32 value)
{
    s32 index = getVoxelIndex(x, y, z);
    c->voxels[index] = value;
}

#endif //VOXEL_H
