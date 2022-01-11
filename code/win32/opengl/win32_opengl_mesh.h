/* date = December 30th 2021 0:05 pm */

#ifndef WIN32_OPENGL_MESH_H
#define WIN32_OPENGL_MESH_H

internal opengl_mesh
opengl_make_mesh(u32 vertexCount, GLsizei vertexSize, memory_arena *arena)
{
    opengl_mesh result = 
    {
        .vertices = push_size(arena, vertexCount*vertexSize, 4),
        .vertexCount = vertexCount,
    };
    return result;
}

internal opengl_mesh_indexed
opengl_make_mesh_indexed(u32 vertexCount, GLsizei vertexSize, u32 indexCount, memory_arena *arena)
{
    opengl_mesh_indexed result = 
    {
        .vertices = push_size(arena, vertexCount*vertexSize, 4),
        .vertexCount = vertexCount,
        .indices = push_array(arena, indexCount, u32, 4),
        .indexCount = indexCount,
    };
    return result;
}

internal opengl_mesh
opengl_make_cube_mesh(v3 offset, v3 scale, memory_arena *arena)
{
    opengl_mesh cube = opengl_make_mesh(3*2*6, sizeof(f32)*8, arena);
    
    f32 minX = offset.x - 0.5f * scale.x;
    f32 maxX = offset.x + 0.5f * scale.x;
    
    f32 minY = offset.y - 0.5f * scale.y;
    f32 maxY = offset.y + 0.5f * scale.y;
    
    f32 minZ = offset.z - 0.5f * scale.z;
    f32 maxZ = offset.z + 0.5f * scale.z;
    
    f32 data[] =
    {
        // Lower face
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,minY,minZ,  2.5f,0.0f,  1,1,1,
        maxX,maxY,minZ,  2.5f,2.5f,  1,1,1,
        
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,minZ,  2.5f,2.5f,  1,1,1,
        minX,maxY,minZ,  0.0f,2.5f,  1,1,1,
        
        // Upper face
        minX,minY,maxZ,  0.0f,0.0f,  1,1,1,
        maxX,minY,maxZ,  2.5f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        
        minX,minY,maxZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        minX,maxY,maxZ,  0.0f,2.5f,  1,1,1,
        
        // Front fa1,1,1e
        minX,maxY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,minZ,  2.5f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        
        minX,maxY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        minX,maxY,maxZ,  0.0f,2.5f,  1,1,1,
        
        // Ba1,1,1k fa1,1,1e
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,minY,minZ,  2.5f,0.0f,  1,1,1,
        maxX,minY,maxZ,  2.5f,2.5f,  1,1,1,
        
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,minY,maxZ,  2.5f,2.5f,  1,1,1,
        minX,minY,maxZ,  0.0f,2.5f,  1,1,1,
        
        // Left fa1,1,1e
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        minX,maxY,minZ,  2.5f,0.0f,  1,1,1,
        minX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        
        minX,minY,minZ,  0.0f,0.0f,  1,1,1,
        minX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        minX,minY,maxZ,  0.0f,2.5f,  1,1,1,
        
        // Right fa1,1,1e
        maxX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,minZ,  2.5f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        
        maxX,minY,minZ,  0.0f,0.0f,  1,1,1,
        maxX,maxY,maxZ,  2.5f,2.5f,  1,1,1,
        maxX,minY,maxZ,  0.0f,2.5f,  1,1,1,
    };
    
    memcpy(cube.vertices, data, sizeof(data));
    
    return cube;
}

internal opengl_mesh_indexed
opengl_make_cube_mesh_indexed(v3 offset, v3 scale, memory_arena *arena, u32 indexBase, u32 textureType)
{
    opengl_mesh_indexed cube = opengl_make_mesh_indexed(4*6, sizeof(f32)*8, 6*6, arena);
    
    f32 minX = offset.x - 0.5f * scale.x;
    f32 maxX = offset.x + 0.5f * scale.x;
    
    f32 minY = offset.y - 0.5f * scale.y;
    f32 maxY = offset.y + 0.5f * scale.y;
    
    f32 minZ = offset.z - 0.5f * scale.z;
    f32 maxZ = offset.z + 0.5f * scale.z;
    
    u32 atlasX = 16;
    u32 atlasY = 16;
    
    f32 du = 1.0f / (f32)atlasX;
    f32 dv = 1.0f / (f32)atlasY;
    
    f32 minU = 0;
    f32 maxU = du;
    
    f32 minV = 0;
    f32 maxV = dv;
    
    if (textureType > 0)
    {
        u32 blockX = textureType;
        u32 blockY = textureType;
        minU = (f32)blockX * du;
        minV = (f32)blockY * dv;
        maxU = minU + du;
        maxV = minV + dv;
    }
    
    f32 g = 0.5f; // gray
    
    f32 vertices[] =
    {
        // Front
        minX, minY, maxZ,  minU, minV, g,g,g, // 0
        maxX, minY, maxZ,  maxU, minV, g,g,g, // 1
        maxX, maxY, maxZ,  maxU, maxV, 1,1,1, // 2
        minX, maxY, maxZ,  minU, maxV, 1,1,1, // 3
        
        // Back
        minX, maxY, minZ,  minU, minV, 1,1,1, // 5
        maxX, maxY, minZ,  maxU, minV, 1,1,1, // 4
        maxX, minY, minZ,  maxU, maxV, g,g,g, // 6
        minX, minY, minZ,  minU, maxV, g,g,g, // 7
        
        // Upper
        minX, maxY, maxZ,  minU, minV, 1,1,1, // 3
        maxX, maxY, maxZ,  maxU, minV, 1,1,1, // 2
        maxX, maxY, minZ,  maxU, maxV, 1,1,1, // 4
        minX, maxY, minZ,  minU, maxV, 1,1,1, // 5
        
        // Lower
        minX, minY, minZ,  minU, minV, g,g,g, // 7
        maxX, minY, minZ,  maxU, minV, g,g,g, // 6
        maxX, minY, maxZ,  maxU, maxV, g,g,g, // 1
        minX, minY, maxZ,  minU, maxV, g,g,g, // 0
        
        // Left
        minX, minY, minZ,  minU, minV, g,g,g, // 7
        minX, minY, maxZ,  maxU, minV, g,g,g, // 0
        minX, maxY, maxZ,  maxU, maxV, 1,1,1, // 3
        minX, maxY, minZ,  minU, maxV, 1,1,1, // 5
        
        // Right
        maxX, minY, maxZ,  minU, minV, g,g,g, // 1
        maxX, minY, minZ,  maxU, minV, g,g,g, // 6
        maxX, maxY, minZ,  maxU, maxV, 1,1,1, // 4
        maxX, maxY, maxZ,  minU, maxV, 1,1,1, // 2
    };
    
    u32 indices[] =
    {
        // Front
        indexBase + 0, indexBase + 1, indexBase + 2, indexBase + 2, indexBase + 3, indexBase + 0,
        
        // Back
        indexBase + 4, indexBase + 5, indexBase + 6, indexBase + 6, indexBase + 7, indexBase + 4,
        
        // Upper
        indexBase + 8, indexBase + 9, indexBase + 10, indexBase + 10, indexBase + 11, indexBase + 8,
        
        // Lower
        indexBase + 12, indexBase + 13, indexBase + 14, indexBase + 14, indexBase + 15, indexBase + 12,
        
        // Left
        indexBase + 16, indexBase + 17, indexBase + 18, indexBase + 18, indexBase + 19, indexBase + 16,
        
        // Right
        indexBase + 20, indexBase + 21, indexBase + 22, indexBase + 22, indexBase + 23, indexBase + 20,
    };
    
    memcpy(cube.vertices, vertices, sizeof(vertices));
    memcpy(cube.indices, indices, sizeof(indices)); 
    
    return cube;
}

internal opengl_mesh_indexed
opengl_make_quad_indexed(memory_arena *arena, v2 minPos, v2 maxPos, v2 minUV, v2 maxUV, u32 indexBase)
{
    opengl_mesh_indexed quad = opengl_make_mesh_indexed(4, sizeof(f32)*8, 6, arena);
    
    f32 minX = minPos.x;
    f32 maxX = maxPos.x;
    
    f32 minY = minPos.y;
    f32 maxY = maxPos.y;
    
    f32 minU = minUV.x;
    f32 minV = minUV.y;
    f32 maxU = maxUV.x;
    f32 maxV = maxUV.y;
    
    f32 vertices[] =
    {
        minX, minY, 0,  minU, minV, 1,1,1, // 0
        maxX, minY, 0,  maxU, minV, 1,1,1, // 1
        maxX, maxY, 0,  maxU, maxV, 1,1,1, // 2
        minX, maxY, 0,  minU, maxV, 1,1,1, // 3
    };
    
    u32 indices[] =
    {
        indexBase + 0, indexBase + 2, indexBase + 1, indexBase + 2, indexBase + 0, indexBase + 3,
    };
    
    memcpy(quad.vertices, vertices, sizeof(vertices));
    memcpy(quad.indices, indices, sizeof(indices)); 
    
    return quad;
}

#endif //WIN32_OPENGL_MESH_H
