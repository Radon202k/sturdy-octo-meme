/* date = May 7th 2022 9:58 pm */

#ifndef VOXEL_H
#define VOXEL_H

#define CHUNK_SIZE v3{16,16,64}

struct chunk
{
    s32 x;
    s32 y;
    s32 z;
    
    u32 *voxels;
    chunk *freeNext;
    chunk *loadedNext;
    chunk *nextInHash;
};

struct noise_hash_node
{
    s32 x;
    s32 y;
    f32 value;
    noise_hash_node *nextInHash;
    noise_hash_node *freeNext;
};

struct voxel_map
{
    hnMandala *permanent;
    
    chunk *hash[2048];
    chunk *freeFirst;
    chunk *loadedFirst;
    
    noise_hash_node *noiseHash[4096];
    noise_hash_node *noiseHashFreeFirst;
    
    f32 viewDist;
    v3 currentCenter; // For chunk loading
    v3 maxDistFromCurrentCenter;
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

internal void
freeChunk(voxel_map *map, chunk *c)
{
    c->freeNext = map->freeFirst;
    map->freeFirst = c;
    
    c->loadedNext = 0;
    c->nextInHash = 0;
    
}


#endif //VOXEL_H
