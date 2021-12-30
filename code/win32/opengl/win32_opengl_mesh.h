/* date = December 30th 2021 0:05 pm */

#ifndef WIN32_OPENGL_MESH_H
#define WIN32_OPENGL_MESH_H

internal opengl_mesh
opengl_make_mesh(u32 vertexCount, memory_arena *arena)
{
    opengl_mesh result = 
    {
        .vertices = push_array(arena, vertexCount, opengl_vertex, 4),
        .vertexCount = vertexCount,
    };
    return result;
}

internal opengl_mesh_indexed
opengl_make_mesh_indexed(u32 vertexCount, u32 indexCount, memory_arena *arena)
{
    opengl_mesh_indexed result = 
    {
        .vertices = push_array(arena, vertexCount, opengl_vertex, 4),
        .vertexCount = vertexCount,
        .indices = push_array(arena, indexCount, u32, 4),
        .indexCount = indexCount,
    };
    return result;
}

internal opengl_mesh
opengl_make_cube_mesh(v3 offset, v3 scale, memory_arena *arena)
{
    opengl_mesh cube = opengl_make_mesh(3*2*6, arena);
    
    f32 minX = offset.x - 0.5f * scale.x;
    f32 maxX = offset.x + 0.5f * scale.x;
    
    f32 minY = offset.y - 0.5f * scale.y;
    f32 maxY = offset.y + 0.5f * scale.y;
    
    f32 minZ = offset.z - 0.5f * scale.z;
    f32 maxZ = offset.z + 0.5f * scale.z;
    
    opengl_vertex data[] =
    {
        // Lower face
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, minY, minZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { maxX, maxY, minZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, minZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { minX, maxY, minZ }, {  0.0f,  2.5f }, {1,1,1} },
        
        // Upper face
        { { minX, minY, maxZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, minY, maxZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { minX, minY, maxZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { minX, maxY, maxZ }, {  0.0f,  2.5f }, {1,1,1} },
        
        // Front fa{1,1,1}e
        { { minX, maxY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, minZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { minX, maxY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { minX, maxY, maxZ }, {  0.0f,  2.5f }, {1,1,1} },
        
        // Ba{1,1,1}k fa{1,1,1}e
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, minY, minZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { maxX, minY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, minY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { minX, minY, maxZ }, {  0.0f,  2.5f }, {1,1,1} },
        
        // Left fa{1,1,1}e
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { minX, maxY, minZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { minX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { minX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { minX, minY, maxZ }, {  0.0f,  2.5f }, {1,1,1} },
        
        // Right fa{1,1,1}e
        { { maxX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, minZ }, {  2.5f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        
        { { maxX, minY, minZ }, {  0.0f,  0.0f }, {1,1,1} },
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {1,1,1} },
        { { maxX, minY, maxZ }, {  0.0f,  2.5f }, {1,1,1} },
    };
    
    memcpy(cube.vertices, data, sizeof(data));
    
    return cube;
}

internal opengl_mesh_indexed
opengl_make_cube_mesh_indexed(v3 offset, v3 scale, memory_arena *arena, u32 indexBase)
{
    opengl_mesh_indexed cube = opengl_make_mesh_indexed(4*6, 6*6, arena);
    
    f32 minX = offset.x - 0.5f * scale.x;
    f32 maxX = offset.x + 0.5f * scale.x;
    
    f32 minY = offset.y - 0.5f * scale.y;
    f32 maxY = offset.y + 0.5f * scale.y;
    
    f32 minZ = offset.z - 0.5f * scale.z;
    f32 maxZ = offset.z + 0.5f * scale.z;
    
    opengl_vertex vertices[] =
    {
        // Front
        { { minX, minY, maxZ }, {  0.0f,  0.0f }, {1,0,0} }, // 0
        { { maxX, minY, maxZ }, {  2.5f,  0.0f }, {0,1,0} }, // 1
        { { maxX, maxY, maxZ }, {  2.5f,  2.5f }, {0,0,1} }, // 2
        { { minX, maxY, maxZ }, {  0.0f,  2.5f }, {1,1,0} }, // 3
        
        // Back
        { { minX, maxY, minZ }, {  0.0f,  0.0f }, {1,0,0} }, // 5
        { { maxX, maxY, minZ }, {  2.5f,  0.0f }, {0,1,0} }, // 4
        { { maxX, minY, minZ }, {  2.5f,  2.5f }, {0,0,1} }, // 6
        { { minX, minY, minZ }, {  0.0f,  2.5f }, {1,1,0} }, // 7
        
        // Upper
        { { minX, maxY, maxZ }, {  0.0f,  0.0f }, {1,0,0} }, // 3
        { { maxX, maxY, maxZ }, {  2.5f,  0.0f }, {0,1,0} }, // 2
        { { maxX, maxY, minZ }, {  2.5f,  2.5f }, {0,0,1} }, // 4
        { { minX, maxY, minZ }, {  0.0f,  2.5f }, {1,1,0} }, // 5
        
        // Lower
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,0,0} }, // 7
        { { maxX, minY, minZ }, {  2.5f,  0.0f }, {0,1,0} }, // 6
        { { maxX, minY, maxZ }, {  2.5f,  2.5f }, {0,0,1} }, // 1
        { { minX, minY, maxZ }, {  0.0f,  2.5f }, {1,1,0} }, // 0
        
        // Left
        { { minX, minY, minZ }, {  0.0f,  0.0f }, {1,0,0} }, // 7
        { { minX, minY, maxZ }, {  2.5f,  0.0f }, {0,1,0} }, // 0
        { { minX, maxY, maxZ }, {  2.5f,  2.5f }, {0,0,1} }, // 3
        { { minX, maxY, minZ }, {  0.0f,  2.5f }, {1,1,0} }, // 5
        
        // Right
        { { maxX, minY, maxZ }, {  0.0f,  0.0f }, {1,0,0} }, // 1
        { { maxX, minY, minZ }, {  2.5f,  0.0f }, {0,1,0} }, // 6
        { { maxX, maxY, minZ }, {  2.5f,  2.5f }, {0,0,1} }, // 4
        { { maxX, maxY, maxZ }, {  0.0f,  2.5f }, {1,1,0} }, // 2
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

#endif //WIN32_OPENGL_MESH_H
