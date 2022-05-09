internal void
freeChunk(mcRenderer *r, voxel_map *map, chunk *c)
{
    freeChunkRenderBuffer(r, c->gpuBuffer);
    
    c->freeNext = map->freeFirst;
    map->freeFirst = c;
}

internal chunk *
makeChunk(mcRenderer *r, voxel_map *map, s32 x, s32 y, s32 z)
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
    
    // TODO: Search free list to remove this from it if it is there.
    
    result->gpuBuffer = makeChunkRenderBuffer(r);
    
    result->p = v3{(f32)x,(f32)y,(f32)z};
    
    result->freeNext = 0;
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
                
                f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compZ,0.05f,6));
                
                if (compZ <= height)
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
getChunk(mcRenderer *r, voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    chunk *result = map->hash[hashIndex];
    
    if (result)
    {
        chunk *lastValidChunkInHash = result;
        while (result && ((result->p.x == x && result->p.y == y && result->p.z == z) == false))
        {
            lastValidChunkInHash = result;
            result = result->nextInHash;
        }
        
        if (result)
        {
            assert((result->p.x == x && result->p.y == y && result->p.z == z));
        }
        else
        {
            // Didn't find the chunk, need to add a new chunk in the hash chain
            chunk *newChunk = makeChunk(r, map, x, 0, z);
            lastValidChunkInHash->nextInHash = newChunk;
            result = newChunk;
        }
    }
    else
    {
        result = makeChunk(r, map, x, y, 0);
    }
    
    
    return result;
}

internal void
freeChunksInRange(mcRenderer *r, voxel_map *map, v3 min, v3 max)
{
    for (s32 chunkX = (s32)min.x;
         chunkX < max.x;
         ++chunkX)
    {
        for (s32 chunkZ = (s32)min.z;
             chunkZ < max.z;
             ++chunkZ)
        {
            chunk *c = getChunk(r, map, chunkX, 0, chunkZ);
            freeChunk(r, map, c);
        }
    }
}

internal b32
hasSpaceAround(chunk *c, s32 x, s32 y, s32 z)
{
    b32 result = false;
    
    // If it is in the boundary of the chunk
    if ((x == 0 || x == CHUNK_SIZE.x) ||
        (y == 0 || y == CHUNK_SIZE.y) ||
        (z == 0 || z == CHUNK_SIZE.z))
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
        else if (getVoxel(c, x, y, z-1) == 0) // Front
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
makeChunksInRange(mcRenderer *r, voxel_map *map, v3 min, v3 max)
{
    // TODO: Y Layers!
    s32 chunkY = 0;
    for (s32 chunkX = (s32)min.x;
         chunkX < max.x;
         ++chunkX)
    {
        for (s32 chunkZ = (s32)min.z;
             chunkZ < max.z;
             ++chunkZ)
        {
            chunk *c = getChunk(r, map, chunkX, 0, chunkZ);
            
            c->gpuBuffer->vb.index = 0;
            c->gpuBuffer->ib.index = 0;
            
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
                        f32 compZ = chunkZ*scale.z*CHUNK_SIZE.z + z;
                        
                        v3 p = 
                        {
                            compX*scale.x,
                            compY*scale.y,
                            compZ*scale.z,
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
                            hnPushCubeIndexed(&c->gpuBuffer->vb, &c->gpuBuffer->ib, sprite, p, scale, hnWHITE);
                        }
                    }
                }
            }
            
            hnUploadGpuBuffer(r->backend, &c->gpuBuffer->vb);
            hnUploadGpuBuffer(r->backend, &c->gpuBuffer->ib);
        }
    }
    
}

internal void
updateChunkLoading(mcRenderer *r, voxel_map *map, v3 camP)
{
    v3 cameraChunkP = floor(camP / CHUNK_SIZE);
    
    if ((map->currentCenter == map->oldCenter) && (map->currentCenter == v3{0,0,0}))
    {
        // First time, load everything
        map->oldCenter = cameraChunkP;
        map->currentCenter = cameraChunkP;
        
        v3 inMin = map->currentCenter - 0.5f*map->viewDist;
        v3 inMax = map->currentCenter + 0.5f*map->viewDist;
        makeChunksInRange(r, map, inMin, inMax);
    }
    else if (absolute(cameraChunkP.x - map->currentCenter.x) >= (0.25f*map->viewDist.x) ||
             absolute(cameraChunkP.z - map->currentCenter.z) >= (0.25f*map->viewDist.z))
    {
        // Unload and load only the chunks around it
        map->oldCenter = map->currentCenter;
        map->currentCenter = cameraChunkP;
        
        v3 oldMin = map->oldCenter - 0.5f*map->viewDist;
        v3 oldMax = map->oldCenter + 0.5f*map->viewDist;
        v3 newMin = map->currentCenter - 0.5f*map->viewDist;
        v3 newMax = map->currentCenter + 0.5f*map->viewDist;
        
        v3 delta = map->currentCenter - map->oldCenter;
        
        v3 inMin = {};
        v3 inMax = {};
        v3 outMin = {};
        v3 outMax = {};
        
        // X
        if (delta.x > 0) 
        {
            inMin.x = oldMax.x;
            outMin.x = oldMin.x;
        }
        else if (delta.x < 0)
        {
            inMin.x = newMin.x;
            outMin.x = newMax.x;
        }
        else
        {
            inMin.x = newMin.x;
            outMin.x = newMin.x;
        }
        
        if (delta.x == 0)
        {
            inMax.x = inMin.x + map->viewDist.x;
            outMax.x = outMin.x + map->viewDist.x;
        }
        else
        {
            inMax.x = inMin.x + 0.25f*map->viewDist.x;
            outMax.x = outMin.x + 0.25f*map->viewDist.x;
        }
        
        // Z
        if (delta.z > 0) 
        {
            inMin.z = oldMax.z;
            outMin.z = oldMin.z;
        }
        else if (delta.z < 0)
        {
            inMin.z = newMin.z;
            outMin.z = newMax.z;
        }
        else
        {
            inMin.z = newMin.z;
            outMin.z = newMin.z;
        }
        
        if (delta.z == 0)
        {
            inMax.z = inMin.z + map->viewDist.z;
            outMax.z = outMin.z + map->viewDist.z;
        }
        else
        {
            inMax.z = inMin.z + 0.25f*map->viewDist.z;
            outMax.z = outMin.z + 0.25f*map->viewDist.z;
        }
        
        // freeChunksInRange(map, outMin, outMax);
        makeChunksInRange(r, map, inMin, inMax);
    }
}
