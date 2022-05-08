/* date = May 7th 2022 9:58 pm */

#ifndef NOISE_H
#define NOISE_H

inline u32
getNoiseHashIndex(voxel_map *map, s32 x, s32 y)
{
    assert(arrayCount(map->noiseHash) % 2 == 0);
    // TODO: Better hash function !!
    u32 hashIndex = (u32)(x * 17 + y * 39);
    return hashIndex % arrayCount(map->noiseHash);
}

inline noise_hash_node *
makeNoiseHashNode(voxel_map *map, s32 x, s32 y)
{
    if (!map->noiseHashFreeFirst)
    {
        map->noiseHashFreeFirst = hnPushStruct(map->permanent, noise_hash_node);
    }
    
    noise_hash_node *result = map->noiseHashFreeFirst;
    map->noiseHashFreeFirst = map->noiseHashFreeFirst->freeNext;
    
    result->x = x;
    result->y = y;
    
    result->value = floorf(64*(f32)hnPerlin2D(x,y,0.05f,6));
    
    return result;
}

internal noise_hash_node *
getNoise(voxel_map *map, s32 x, s32 y)
{
    u32 hashIndex = getNoiseHashIndex(map,x,y); 
    
    noise_hash_node *result = map->noiseHash[hashIndex];
    
    if (result)
    {
        noise_hash_node *lastValid = result;
        while (result && !(result->x == x && result->y == y))
        {
            lastValid = result;
            result = result->nextInHash;
        }
        
        if (result)
        {
            assert((result->x == x && result->y == y));
        }
        else
        {
            // didnt find one, has to make it
            result = makeNoiseHashNode(map, x, y);
        }
    }
    else
    {
        result = makeNoiseHashNode(map, x, y);
    }
    
    return result;
}

internal void
setNoise(voxel_map *map, s32 x, s32 y, f32 value)
{
    u32 hashIndex = getNoiseHashIndex(map,x,y); 
    
    noise_hash_node *node = getNoise(map, x, y);
    assert(node);
    
    node->value = value;
}

internal void
generateNoise2DAroundP(voxel_map *map, v3 center, f32 dist, f32 maxHeight)
{
    s32 centerChunkX = (s32)(center.x / CHUNK_SIZE.x);
    s32 centerChunkY = (s32)(center.z / CHUNK_SIZE.y);
    
    // TODO: Z Layers!
    s32 chunkZ = 0;
    for (s32 chunkX = centerChunkX - (s32)dist;
         chunkX < centerChunkX + dist;
         ++chunkX)
    {
        for (s32 chunkY = centerChunkY - (s32)dist;
             chunkY < centerChunkY + dist;
             ++chunkY)
        {
            for (s32 voxelZ = 0;
                 voxelZ < CHUNK_SIZE.z;
                 ++voxelZ)
            {
                for (s32 voxelY = 0;
                     voxelY < CHUNK_SIZE.y;
                     ++voxelY)
                {
                    for (s32 voxelX = 0;
                         voxelX < CHUNK_SIZE.x;
                         ++voxelX)
                    {
                        u32 cubeMaterial = 1;
                        
                        f32 compX = (f32)(chunkX*CHUNK_SIZE.x + voxelX);
                        f32 compY = (f32)(chunkY*CHUNK_SIZE.y + voxelY);
                        f32 compZ = (f32)(voxelZ);
                        
                        f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compY,0.05f,6));
                        setNoise(map, (s32)compX, (s32)compY, height);
                    }
                }
            }
        }
    }
}

#endif //NOISE_H
