internal void
freeLoadedChunksAtOnce(voxel_map *map)
{
    chunk *c = map->loadedFirst;
    while (c)
    {
        freeChunk(map, c);
        c = c->loadedNext;
    }
    map->loadedFirst = 0;
    
}

internal chunk *
generateChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    if (!map->freeFirst)
    {
        map->freeFirst = hnPushStruct(map->permanent, chunk);
        
        s32 voxelCount = (s32)(CHUNK_SIZE.x*CHUNK_SIZE.y*CHUNK_SIZE.z);
        map->freeFirst->voxels = hnPushArray(map->permanent, voxelCount, u32);
    }
    
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    map->hash[hashIndex] = map->freeFirst;
    map->freeFirst = map->freeFirst->freeNext;
    
    chunk *result = map->hash[hashIndex];
    
    result->x = x;
    result->y = y;
    result->z = z;
    
    result->freeNext = 0;
    result->loadedNext = 0;
    result->nextInHash = 0;
    
    f32 maxHeight = 64;
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
                
                f32 compX = (f32)(x*CHUNK_SIZE.x + voxelX);
                f32 compY = (f32)(y*CHUNK_SIZE.y + voxelY);
                f32 compZ = (f32)(z*CHUNK_SIZE.z + voxelZ);
                
                noise_hash_node *height = getNoise(map, (s32)compX, (s32)compY);
                
                if (compZ <= height->value)
                {
                    if (compZ > maxHeight/1.5f)
                    {
                        cubeMaterial = 3;
                    }
                    else if (compZ > maxHeight/2)
                    {
                        cubeMaterial = 2;
                    }
                    
                    setVoxel(result, voxelX, voxelY, voxelZ, cubeMaterial);
                }
                else
                {
                    setVoxel(result, voxelX, voxelY, voxelZ, 0);
                }
            }
        }
    }
    
    return result;
}

inline chunk *
getChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    chunk *result = map->hash[hashIndex];
    
    if (result)
    {
        chunk *lastValidChunkInHash = result;
        while (result && ((result->x == x && result->y == y && result->z == z) == false))
        {
            lastValidChunkInHash = result;
            result = result->nextInHash;
        }
        
        if (result)
        {
            assert((result->x == x && result->y == y && result->z == z));
        }
        else
        {
            // Didn't find the chunk, need to add a new chunk in the hash chain
            chunk *newChunk = generateChunk(map, x, y, 0);
            lastValidChunkInHash->nextInHash = newChunk;
            result = newChunk;
        }
    }
    else
    {
        result = generateChunk(map, x, y, 0);
    }
    
    result->loadedNext = map->loadedFirst;
    map->loadedFirst = result;
    
    return result;
}

internal b32
hasSpaceAround(chunk *c, s32 x, s32 y, s32 z)
{
    b32 result = false;
    
    // If it is in the boundary of the chunk
    if ((x == 0 || x == 7) || (y == 0 || y == 7) || (z == 0 || z == 7))
    {
        result = true;
    }
    else
    {
        if (getVoxel(c, x-1, y, z) == 0) // Left
        {
            result = true;
        }
        else if (getVoxel(c, x+1, y, z) == 0) // Right
        {
            result = true;
        }
        else if (getVoxel(c, x, y-1, z) == 0) // Below
        {
            result = true;
        }
        else if (getVoxel(c, x, y+1, z) == 0) // Above
        {
            result = true;
        }
        else if (getVoxel(c, x, y, z-1) == 0) // Font
        {
            result = true;
        }
        else if (getVoxel(c, x, y, z+1) == 0) // Back
        {
            result = true;
        }
    }
    
    return result;
}


internal void
generateChunksAroundP(mcRenderer *r, voxel_map *map, v3 center, f32 dist)
{
    r->cubes.vb.index = 0;
    r->cubes.ib.index = 0;
    
    s32 centerChunkX = (s32)(center.x / CHUNK_SIZE.x);
    s32 centerChunkY = (s32)(center.z / CHUNK_SIZE.y);
    
    freeLoadedChunksAtOnce(map);
    
    // TODO: Z Layers!
    for (s32 chunkX = centerChunkX - (s32)dist;
         chunkX < centerChunkX + dist;
         ++chunkX)
    {
        for (s32 chunkY = centerChunkY - (s32)dist;
             chunkY < centerChunkY + dist;
             ++chunkY)
        {
            chunk *c = getChunk(map, chunkX, chunkY, 0);
            
            v3 scale = v3{1,1,1};
            for (s32 z = 0;
                 z < CHUNK_SIZE.z;
                 ++z)
            {
                for (s32 y = 0;
                     y < CHUNK_SIZE.y;
                     ++y)
                {
                    for (s32 x = 0;
                         x < CHUNK_SIZE.x;
                         ++x)
                    {
                        u32 material = getVoxel(c, x, y, z);
                        
                        f32 compX = chunkX*scale.x*CHUNK_SIZE.x + x;
                        f32 compY = chunkY*scale.y*CHUNK_SIZE.y + y;
                        f32 compZ = (f32)z;
                        
                        v3 p = 
                        {
                            compX*scale.x,
                            compZ*scale.z,
                            compY*scale.y
                        };
                        
                        hnSprite sprite = r->white;
                        v4 color = {};
                        if (material == 1)
                        {
                            sprite = r->dirt;
                        }
                        else if (material == 2)
                        {
                            sprite = r->stone;
                        }
                        else if (material == 3)
                        {
                            sprite = r->snow;
                        }
                        
                        if (material && hasSpaceAround(c, x, y, z))
                        {
                            hnPushCubeIndexed(&r->cubes.vb, &r->cubes.ib, sprite, p, scale, hnWHITE);
                        }
                    }
                }
            }
        }
    }
    
    hnUploadGpuBuffer(r->backend, &r->cubes.vb);
    hnUploadGpuBuffer(r->backend, &r->cubes.ib);
}

internal void
updateChunkLoading(mcRenderer *r, voxel_map *map, v3 camP)
{
    f32 xDiff = camP.x - map->currentCenter.x;
    f32 zDiff = camP.z - map->currentCenter.z;
    
    if (absolute(xDiff) > map->maxDistFromCurrentCenter.x ||
        absolute(zDiff) > map->maxDistFromCurrentCenter.z)
    {
        map->currentCenter = camP;
        
        generateChunksAroundP(r, map, camP, map->viewDist);
    }
}
