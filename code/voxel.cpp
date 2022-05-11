internal void
freeChunk(mcRenderer *r, voxel_map *map, chunk *c)
{
    assert(c->active);
    assert(c->vb);
    assert(c->ib);
    
    c->active = false;
    
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
    // Assume chunk came from free list 
    b32 cameFromFreeList = true;
    
    if (!map->freeFirst)
    {
        // Only when the free list is empty it means the chunk didn't come from it
        cameFromFreeList = false;
        
        s32 voxelCount = (s32)(CHUNK_SIZE.x*CHUNK_SIZE.y*CHUNK_SIZE.z);
        // Allocate memory for brand new chunk
        map->freeFirst = hnPushStruct(map->permanent, chunk);
        map->freeFirst->voxels = hnPushArray(map->permanent, voxelCount, u32);
        // Clear voxel memory to zero
        memset(map->freeFirst->voxels, 0, voxelCount*4);
    }
    
    v3 oldP = map->freeFirst->p;
    v3 newP = v3{(f32)x,(f32)y,(f32)z};
    u32 hashIndex = getChunkHashIndex(map, (s32)newP.x, (s32)newP.y, (s32)newP.z);
    
    // Grab the head of the free list for the new chunk
    chunk *result = map->freeFirst;
    map->freeFirst = map->freeFirst->freeNext;
    
    if (cameFromFreeList)
    {
        // If this chunk came from the free list, it means it is in the hash table
        // as an inactive chunk waiting to be used. Now, since we are getting it from here
        // it means it should have a different position than the one we want.
        assert((oldP != newP));
        assert(result->active == false);
        assert(result->vb && result->ib);
        
        // Also, the chunk is currently in a wrong position in the hash table, so we need
        // to remove it from that chain and push it to a new chain. This is because we are
        // reusing this chunk, so we are updating the pointers now.
        if (result->prevInHash)
        {
            // Update next pointer of the chunk previous to this in the chain if there is one
            result->prevInHash->nextInHash = result->nextInHash;
        }
        else
        {
            // If there is not previous in hash, it means this chunk was the head of the chain
            // In that case we need to remove it from the head as well.
            u32 oldHashIndex = getChunkHashIndex(map, (s32)oldP.x, (s32)oldP.y, (s32)oldP.z);
            assert(map->hash[oldHashIndex] == result);
            
            map->hash[oldHashIndex] = result->nextInHash;
        }
        
        if (result->nextInHash)
        {
            // Update prev pointer of the chunk next to this in the chain if there is one
            result->nextInHash->prevInHash = result->prevInHash;
        }
    }
    
    // Now put the new chunk in the head of the new hash chain it belongs to
    if (map->hash[hashIndex])
    {
        map->hash[hashIndex]->prevInHash = result;
    }
    result->prevInHash = 0;
    result->nextInHash = map->hash[hashIndex];
    map->hash[hashIndex] = result;
    
    if (!result->vb)
    {
        assert(!result->ib);
        
        /* NOTE: 
        
A chunk is 16x16x16 blocks = 4096 blocks
However at maximum only half of the blocks can have 
up to 3 maximum faces visible at the same time.
 So we can assume the vertex buffer will never need
more than 2048x3 = 6144 faces worth of space.
we have 36 bytes per vertex, 4 vertices per face
that is 36x4x6144 = 884kb max of gpu memory.


Similarly, 4 bytes x 6 indices x 6144 faces = 147kb (index buffer)

IMPORTANT: Since we push faces one at a time, we
may have a lot of redundent vertices being pushed.
TODO: So we should consider to either try to reuse
vertices or just stop using indices altogether?

*/
        
        // Make vertex and index buffers if this chunk doesn't have any yet
        result->vb = hnMakeVertexBuffer(r->backend, (u32)kilobytes(884), sizeof(f32)*9); 
        result->ib = hnMakeIndexBuffer(r->backend, kilobytes(147));
        
        hnSetInputLayout(result->vb, 0, GL_FLOAT, 3, offsetof(vertex3d, pos));
        hnSetInputLayout(result->vb, 1, GL_FLOAT, 3, offsetof(vertex3d, uv));
        hnSetInputLayout(result->vb, 2, GL_FLOAT, 3, offsetof(vertex3d, nor));
    }
    
    // The chunk will only become active after the generation function
    // takes care of it. Right now it is marked as not active.
    result->active = false;
    
    // We don't set the position in make chunk, so that we can check
    // against it outside of this function, to know if we need to recreate
    // the chunk data or not.
    
    result->cameFromFreeList = cameFromFreeList;
    
    result->freeNext = 0;
    result->freePrev = 0;
    
    return result;
}

internal void
removeChunkIfInFreeList(mcRenderer *r, voxel_map *map, chunk *c)
{
    chunk *freeNode = map->freeFirst;
    while (freeNode)
    {
        // Remove the chunk itself from chunk free list if the pos is the same
        if (freeNode->p == c->p)
        {
            // If there is a previous pointer for this free node
            if (freeNode->freePrev)
            {
                // Update it to point to this node's next pointer
                freeNode->freePrev->freeNext = freeNode->freeNext;
            }
            else
            {
                // If there was not a previous pointer to this node
                // it means that this node was the head of the free list
                assert(map->freeFirst == freeNode);
                
                // So it needs to be removed from the head of the free list
                map->freeFirst = freeNode->freeNext;
            }
            
            if (freeNode->freeNext)
            {
                // Update free next's previous pointer if there is a free next
                freeNode->freeNext->freePrev = freeNode->freePrev;
            }
            
            freeNode->freePrev = 0;
            freeNode->freeNext = 0;
            
            // There should be only one entry for chunk in the free list so can early exit
            break;
        }
        
        freeNode = freeNode->freeNext;
    }
}

inline chunk *
getChunk(voxel_map *map, s32 x, s32 y, s32 z)
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
    }
    
    assert((result == 0) || (result->p.x == x && result->p.y == y && result->p.z == z));
    
    return result;
}

inline s32
getVoxel(voxel_map *map, chunk *c, s32 x, s32 y, s32 z)
{
    s32 chunkX = (s32)c->p.x;
    s32 chunkY = (s32)c->p.y;
    s32 chunkZ = (s32)c->p.z;
    b32 changedChunk = false;
    if (x < 0)
    {
        --chunkX;
        x += (s32)CHUNK_SIZE.x-1;
        changedChunk = true;
    }
    else if (x >= CHUNK_SIZE.x)
    {
        ++chunkX;
        x -= (s32)CHUNK_SIZE.x-1;
        changedChunk = true;
    }
    
    if (y < 0)
    {
        --chunkY;
        y += (s32)CHUNK_SIZE.y-1;
        changedChunk = true;
    }
    else if (y >= CHUNK_SIZE.y)
    {
        ++chunkY;
        y -= (s32)CHUNK_SIZE.y-1;
        changedChunk = true;
    }
    
    if (z < 0)
    {
        --chunkZ;
        z += (s32)CHUNK_SIZE.z-1;
        changedChunk = true;
    }
    else if (z >= CHUNK_SIZE.z)
    {
        ++chunkZ;
        z -= (s32)CHUNK_SIZE.z-1;
        changedChunk = true;
    }
    
    s32 result = 0;
    if (changedChunk)
    {
        c = getChunk(map, chunkX, chunkY, chunkZ);
    }
    
    if (c)
    {
        s32 index = getVoxelIndex(x, y, z);
        result = c->voxels[index];
    }
    
    return result;
}

inline void
setVoxel(chunk *c, s32 x, s32 y, s32 z, u32 value)
{
    s32 index = getVoxelIndex(x, y, z);
    c->voxels[index] = value;
}

internal void
pushFacesThatFaceAFreeSpace(voxel_map *map, chunk *c, hnSprite sprite, s32 x, s32 y, s32 z)
{
    v3 pos = c->p * CHUNK_SIZE + v3{(f32)x,(f32)y,(f32)z};
    v3 dim = {1,1,1};
    
    v3 uvMin = sprite.uvMin;
    v3 uvMax = sprite.uvMax;
    
    f32 nx = pos.x - 0.5f*dim.x; 
    f32 ny = pos.y - 0.5f*dim.y;
    f32 nz = pos.z - 0.5f*dim.z;
    
    f32 px = pos.x + 0.5f*dim.x; 
    f32 py = pos.y + 0.5f*dim.y;
    f32 pz = pos.z + 0.5f*dim.z;
    
    v4 color = hnWHITE;
    
    
#if 0
    if (y == c->maxHeights[z][x]-1)
    {
        // Up face
        hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,py,pz},{px,py,pz},{px,py,nz},{nx,py,nz},{0,1,0});
    }
    else if (y == 0)
    {
        // TODO: Draw bottom face?
        // hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,ny,nz},{px,ny,nz},{px,ny,pz},{nx,ny,pz},{0,-1,0});
    }
#endif
    
    if (c)
    {
        if (getVoxel(map, c, x-1, y, z) == 0)
        {
            // Left face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,ny,nz},{nx,ny,pz},{nx,py,pz},{nx,py,nz},{-1,0,0});
        }
        
        if (getVoxel(map, c, x+1, y, z) == 0)
        {
            // Right face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{px,ny,pz},{px,ny,nz},{px,py,nz},{px,py,pz},{1,0,0});
        }
        
        if (getVoxel(map, c, x, y-1, z) == 0)
        {
            // Bottom face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,ny,nz},{px,ny,nz},{px,ny,pz},{nx,ny,pz},{0,-1,0});
        }
        
        if (getVoxel(map, c, x, y+1, z) == 0)
        {
            // Up face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,py,pz},{px,py,pz},{px,py,nz},{nx,py,nz},{0,1,0});
        }
        
        if (getVoxel(map, c, x, y, z-1) == 0)
        {
            // Back face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,py,nz},{px,py,nz},{px,ny,nz},{nx,ny,nz},{0,0,-1});
        }
        
        if (getVoxel(map, c, x, y, z+1) == 0)
        {
            // Front face
            hnPushCubeFaceIndexed(c->vb,c->ib,sprite,{nx,ny,pz},{px,ny,pz},{px,py,pz},{nx,py,pz},{0,0,1});
        }
    }
}

internal void
generateChunkValues(chunk *c, f32 maxHeight)
{
    // Determine max height per voxel (x,z)
    memset(c->maxHeights, 0, (s32)CHUNK_SIZE.z*(s32)CHUNK_SIZE.x);
    
    for (s32 voxelX = 0;
         voxelX < CHUNK_SIZE.x;
         ++voxelX)
    {
        for (s32 voxelZ = 0;
             voxelZ < CHUNK_SIZE.z;
             ++voxelZ)
        {
            u32 cubeMaterial = 1;
            
            f32 compX = (f32)(c->p.x*CHUNK_SIZE.x + voxelX);
            f32 compZ = (f32)(c->p.z*CHUNK_SIZE.z + voxelZ);
            
            f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compZ,0.009f,4));
            c->maxHeights[voxelZ][voxelX] = height;
        }
    }
    
    // Set every voxel below maxheight to zero
    for (s32 voxelX = 0;
         voxelX < CHUNK_SIZE.x;
         ++voxelX)
    {
        for (s32 voxelZ = 0;
             voxelZ < CHUNK_SIZE.z;
             ++voxelZ)
        {
            s32 maxHeightForXZ = (s32)c->maxHeights[voxelZ][voxelX];
            for (s32 voxelY = 0;
                 voxelY < CHUNK_SIZE.y;
                 ++voxelY)
            {
                s32 compY = (s32)c->p.y * (s32)CHUNK_SIZE.y + voxelY;
                
                if (compY < maxHeightForXZ)
                {
                    setVoxel(c, voxelX, voxelY, voxelZ, 1);
                }
                else
                {
                    setVoxel(c, voxelX, voxelY, voxelZ, 0);
                }
            }
        }
    }
}

internal void
pushChunkGeometryToGpuBuffer(mcRenderer *r, voxel_map *map, chunk *c)
{
    c->vb->index = 0;
    c->ib->index = 0;
    
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
                u32 material = getVoxel(map, c, x, y, z);
                
                f32 compX = c->p.x*scale.x*CHUNK_SIZE.x + x;
                f32 compY = c->p.y*scale.y*CHUNK_SIZE.y + y;
                f32 compZ = c->p.z*scale.z*CHUNK_SIZE.z + z;
                
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
                
                if (material)
                {
                    pushFacesThatFaceAFreeSpace(map, c, sprite, x, y, z);
                }
            }
        }
    }
    
    hnUploadGpuBuffer(r->backend, c->vb);
    hnUploadGpuBuffer(r->backend, c->ib);
}

internal void
updateChunkLoading(mcRenderer *r, voxel_map *map, v3 camP)
{
    v3 cameraChunkP = floor(camP / CHUNK_SIZE);
    
    b32 firstTime = ((map->currentCenter == map->oldCenter) && (map->currentCenter == v3{0,0,0}));
    
    b32 camMovedMaxDistance = (length(cameraChunkP - map->currentCenter) >= 0.25f*map->viewDist.x);
    if (firstTime || camMovedMaxDistance)
    {
        // Unload and load only the chunks around it
        map->oldCenter = map->currentCenter;
        map->currentCenter = cameraChunkP;
        
        // NOTE: Determine the minimum and maximum bounds between the old
        // and the new center position (based on view dist).
        
        v3 p[4] = {};
        p[0] = map->oldCenter - 0.5f*map->viewDist; // oldMin
        p[1] = map->oldCenter + 0.5f*map->viewDist; // oldMax
        p[2] = map->currentCenter - 0.5f*map->viewDist; // newMin
        p[3] = map->currentCenter + 0.5f*map->viewDist; // newMax
        
        v3 min = {10000000,10000000,10000000};
        v3 max = {-10000000,-10000000,-10000000};
        
        for (s32 i = 0;
             i < 4;
             ++i)
        {
            // Determine min
            if (p[i].x < min.x){ min.x = p[i].x; }
            if (p[i].y < min.y){ min.y = p[i].y; }
            if (p[i].z < min.z){ min.z = p[i].z; }
            
            // Determine max
            if (p[i].x > max.x){ max.x = p[i].x; }
            if (p[i].y > max.y){ max.y = p[i].y; }
            if (p[i].z > max.z){ max.z = p[i].z; }
        }
        
        // NOTE: Loop through all of the chunks in a big square containing all
        // chunks that could be inside the circle with viewdist as radius. They
        // are tested against a dist diagonally so we have a "circular" view dist.
        for (s32 chunkX = (s32)(min.x + 0.5f);
             chunkX < (s32)(max.x + 0.5f);
             ++chunkX)
        {
            for (s32 chunkY = (s32)(min.y + 0.5f);
                 chunkY < (s32)(max.y + 0.5f);
                 ++chunkY)
            {
                for (s32 chunkZ = (s32)(min.z + 0.5f);
                     chunkZ < (s32)(max.z + 0.5f);
                     ++chunkZ)
                {
                    v3 chunkP = {(f32)chunkX,(f32)chunkY,(f32)chunkZ};
                    
                    v3 newCenter = map->currentCenter;
                    
                    f32 distFromNewCenter = length(newCenter - chunkP); // Integer chunk coords
                    // NOTE: View dist is a v3 with all equal dimensions
                    f32 maxDist = 0.5f * map->viewDist.x;  
                    
                    // NOTE: Free the chunk if it is too distant (outside the "circle")
                    if (distFromNewCenter > maxDist)
                    {
                        chunk *c = getChunk(map, chunkX, chunkY, chunkZ);
                        if (c && c->active)
                        {
                            freeChunk(r, map, c);
                        }
                    }
                    else
                    {
                        // NOTE: The chunk is inside the "circle"
                        chunk *c = getChunk(map, chunkX, chunkY, chunkZ);
                        
                        // NOTE: If the hash table look up failed, it means this chunk has not been created yet.
                        // So we will need to make it.
                        if (!c)
                        {
                            c = makeChunk(r, map, chunkX, chunkY, chunkZ);
                            assert(c->vb && c->ib);
                            
                            if (c->cameFromFreeList)
                            {
                                // NOTE: If the chunk came from the free list, it should never be the same chunk
                                assert(c->p != chunkP);
                                // This could never happen because when we free a chunk we let it keep its pos
                                // and all the rest of its data, that is so that we can keep that work later on.
                                // So in the hash table, the chunk is marked as inactive and the chunk is also
                                // pushed to the free list, but the hash table still contains the pointer to the
                                // freed chunk, which means we can access it directly and through the free list.
                                // But since we always try to getChunk() before calling makeChunk() (which uses
                                // the free list), it means we will always get the chunk in getChunk() if it has
                                // been created before, which is the only case it could have the same position.
                                // Then, we also remove it from the free list right there, so the assert should
                                // never fail.
                                
                                assert(c->vb && c->ib);
                                c->vb->index = 0;
                                c->ib->index = 0;
                            }
                            else
                            {
                                assert(!c->vb->index && !c->ib->index);
                            }
                            
                            // NOTE: makeChunk() doesn't set the position because of the assert above, so we need
                            // to set it right here.
                            c->p = v3{(f32)chunkX,(f32)chunkY,(f32)chunkZ};
                            
                        }
                        
                        if (c->active == false)
                        {
                            c->active = true;
                            
                            // NOTE: If this chunk in in the free list currently that is a problem
                            // Because since we are accessing this chunk directly in the hash table
                            // we wouldn't know it is used when we would try to use it from the free
                            // list. We could choose to test against it then and remove it from the
                            // free list, or we could choose to remove it from the free list here.
                            // Currently we are just removing it from the free list here.
                            removeChunkIfInFreeList(r, map, c);
                            // TODO: Test with both options, and measure which one performs better.
                            
                            
                            // TODO: We have the chunk generation and the data copy to GPU in 
                            // separate steps at the moment. Not sure if it is a good idea?
                            // There are possibly redundent loops going on.
                            generateChunkValues(c, 128);
                            pushChunkGeometryToGpuBuffer(r, map, c);
                        }
                        else
                        {
                            assert(c->vb && c->ib);
                            if (!c->vb->index && !c->ib->index)
                            {
                                // NOTE: Completely blank active chunk
                            }
                        }
                    }
                }
            }
        }
    }
}