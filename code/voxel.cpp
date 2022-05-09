internal void
freeChunk(mcRenderer *r, voxel_map *map, chunk *c)
{
    freeChunkRenderBuffer(r, c->gpuBuffer);
    
    c->gpuBuffer = 0;
    
    // Add chunk to free list
    if (map->freeFirst)
    {
        map->freeFirst->freePrev = c;
        c->freeNext = map->freeFirst;
    }
    else
    {
        c->freeNext = 0;
    }
    
    c->freePrev = 0;
    
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
    
    chunk *result = map->freeFirst;
    map->freeFirst = map->freeFirst->freeNext;
    
    // Remove chunk from hash table in case this came from the free list
    if (result->prevInHash)
    {
        result->prevInHash->nextInHash = result->nextInHash;
    }
    else
    {
        u32 oldHashIndex = getChunkHashIndex(map, (s32)result->p.x, (s32)result->p.y, (s32)result->p.z);
        if (map->hash[oldHashIndex] == result)
        {
            map->hash[oldHashIndex] = 0;
        }
    }
    
    if (result->nextInHash)
    {
        result->nextInHash->prevInHash = result->prevInHash;
    }
    result->prevInHash = 0;
    result->nextInHash = 0;
    
    
    // Put in the head of the hash chain
    if (map->hash[hashIndex])
    {
        map->hash[hashIndex]->prevInHash = result;
    }
    
    result->nextInHash = map->hash[hashIndex];
    map->hash[hashIndex] = result;
    
    if (!result->gpuBuffer)
    {
        // Make vertex and index buffers
        result->gpuBuffer = makeChunkRenderBuffer(r);
    }
    else
    {
        result->gpuBuffer->active = true;
    }
    
    result->p = v3{(f32)x,(f32)y,(f32)z};
    
    result->freeNext = 0;
    result->freePrev = 0;
    
    return result;
}

inline chunk *
getChunk(mcRenderer *r, voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    chunk *result = map->hash[hashIndex];
    
    if (result)
    {
        // Advance through the hash list until find a chunk with the right pos
        while (result && ((result->p.x == x && result->p.y == y && result->p.z == z) == false))
        {
            result = result->nextInHash;
        }
        
        // If found the chunk
        if (result)
        {
            assert((result->p.x == x && result->p.y == y && result->p.z == z));
            
            chunk *c = map->freeFirst;
            while (c)
            {
                if (c->p == result->p)
                {
                    if (c->freePrev)
                    {
                        c->freePrev->freeNext = c->freeNext;
                    }
                    else
                    {
                        if (map->freeFirst == c)
                        {
                            map->freeFirst = 0;
                        }
                    }
                    
                    if (c->freeNext)
                    {
                        c->freeNext->freePrev = c->freePrev;
                    }
                    c->freePrev = 0;
                    c->freeNext = 0;
                    
                    
                    break;
                }
                
                c = c->freeNext;
            }
        }
        else
        {
            // Didn't find the chunk, need to add a new chunk in the hash chain
            chunk *newChunk = makeChunk(r, map, x, y, z);
            result = newChunk;
        }
    }
    else
    {
        result = makeChunk(r, map, x, y, z);
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
    if ((x == 0 || x == CHUNK_SIZE.x-1) ||
        (y == 0 || y == CHUNK_SIZE.y-1) ||
        (z == 0 || z == CHUNK_SIZE.z-1))
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
    // First generate all the cubes
    f32 maxHeight = 64;
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
            
            for (s32 voxelY = 0;
                 voxelY < CHUNK_SIZE.y;
                 ++voxelY)
            {
                for (s32 voxelX = 0;
                     voxelX < CHUNK_SIZE.x;
                     ++voxelX)
                {
                    for (s32 voxelZ = 0;
                         voxelZ < CHUNK_SIZE.z;
                         ++voxelZ)
                    {
                        u32 cubeMaterial = 1;
                        
                        f32 compX = (f32)(chunkX*CHUNK_SIZE.x + voxelX);
                        f32 compY = (f32)(chunkY*CHUNK_SIZE.y + voxelY);
                        f32 compZ = (f32)(chunkZ*CHUNK_SIZE.z + voxelZ);
                        
                        f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compZ,0.05f,6));
                        
                        if (compY <= height)
                        {
                            if (compZ > maxHeight/1.5f)
                            {
                                cubeMaterial = 3;
                            }
                            else if (compZ > maxHeight/2)
                            {
                                cubeMaterial = 2;
                            }
                            
                            setVoxel(c, voxelX, voxelY, voxelZ, cubeMaterial);
                        }
                        else
                        {
                            setVoxel(c, voxelX, voxelY, voxelZ, 0);
                        }
                    }
                }
            }
        }
    }
    
    // Then push only the cubes that should be drawn
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
            for (s32 y = 0;
                 y < CHUNK_SIZE.y;
                 ++y)
            {
                for (s32 x = 0;
                     x < CHUNK_SIZE.x;
                     ++x)
                {
                    for (s32 z = 0;
                         z < CHUNK_SIZE.z;
                         ++z)
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
        
        freeChunksInRange(r, map, outMin, outMax);
        makeChunksInRange(r, map, inMin, inMax);
    }
}
