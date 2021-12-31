#define OPENGL_RENDERER
#include "win32/win32_core.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

WIN32_ENTRY()
{
    win32_init();
    
    memory_arena *arena = &os.mainArena;
    
    opengl_init(&os.gl, 0, 0, 500, 500, "Minecraft clone", arena);
    
    
    
    int x,y,n;
    unsigned char *data = stbi_load("atlas.png", &x, &y, &n, 0);
    if (!data)
    {
        fatal_error("Failed to open image.");
    }
    
    opengl_make_texture(&os.gl.texture, x, y, (u32 *)data);
    stbi_image_free(data);
    
    os.window_is_open = 1;
    
    LARGE_INTEGER perfFrequency;
    QueryPerformanceFrequency(&perfFrequency);
    os.perfFrequency = (f32)perfFrequency.QuadPart;
    
    transform_t cameraTarget = transform_get_default();
    cameraTarget.translation.y = 2;
    cameraTarget.rotation.z = 0.5f*3.1415f;
    camera_t camera = 
    {
        .pos = {0,0,0},
        .pitch = 0,
        .yaw = 0,
        .roll = 0,
        
        .targetTransform = &cameraTarget,
        
        .distanceFromTarget = 50,
        .angleAroundTarget = 0,
    };
    
    v2 mousePos = {.x = os.mouse.pos.x, .y = os.mouse.pos.y};
    
    v2 lastMousePos = mousePos;
    
    // Make vertex buffer array
    os.gl.vertexBuffers = make_typeless_vector(2, sizeof(opengl_vertexbuffer));
    
    opengl_vertexbuffer *cubesVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[0];
    opengl_vertexbuffer *movingVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[1];
    
    /* 

NOTE:

- First vertex buffer for cubes
     - Second for moving objects

*/
    
    // First vertex buffer, static cubes
    // generation code
    {
        u32 maxVertexCount = 500000;
        u32 maxIndexCount = 500000;
        u32 vertexCount = 0;
        u32 indexCount = 0;
        
        opengl_vertex *tempVertices = push_array(arena, maxVertexCount, opengl_vertex, 4);
        u32 *tempIndices = push_array(arena, maxIndexCount, u32, 4);
        
        s32 range = 16;
        
        for (s32 j = -range;
             j < range;
             ++j)
        {
            for (s32 i = -range;
                 i < range;
                 ++i)
            {
                f32 scale = 1.8f;
                v3 size = {scale, scale, scale};
                
                s32 height = 5  + (s32)(-10.0f * (0.707f + perlin2d((f32)i, (f32)j, 0.009f, 4)) / (0.707f*2));
                
                
                v3 offset = {(f32)i * size.x, (f32)height * size.y, (f32)j * size.z};
                opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, vertexCount, 0);
                
                memcpy(tempVertices + vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                memcpy(tempIndices + indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                
                // Register the amount of vertices added
                vertexCount += cube.vertexCount;
                indexCount += cube.indexCount;
            }
        }
        
        *cubesVertexBuffer = opengl_make_vertexbuffer();
        opengl_upload_vertexbuffer_data_immutable(cubesVertexBuffer, vertexCount, tempVertices, indexCount, tempIndices);
    }
    
    // Second vertex buffer, moving objects
    *movingVertexBuffer = opengl_make_vertexbuffer();
    
    
    // Main loop
    LARGE_INTEGER lastCounter = win32_get_perfcounter();
    while(os.window_is_open)
    {
        LARGE_INTEGER counter = win32_get_perfcounter();
        f32 elapsedTime = win32_get_elapsed_time(lastCounter, counter);
        lastCounter = counter;
        
        // Handle OS events
        win32_poll_messages();
        
        mousePos.x = os.mouse.pos.x;
        mousePos.y = os.mouse.pos.y;
        v2 deltaMousePos = v2_sub(mousePos, lastMousePos);
        lastMousePos = mousePos;
        
        if (os.mouse.buttons[0].down)
        {
            if (os.keyboard.buttons[KEY_CONTROL].down)
            {
                camera.distanceFromTarget += elapsedTime * deltaMousePos.y;
            }
            else if (os.keyboard.buttons[KEY_SPACE].down)
            {
            }
            else
            {
                camera.pitch += elapsedTime * 0.1f * deltaMousePos.y;
                camera.angleAroundTarget += 0.1f * elapsedTime * deltaMousePos.x;
            }
        }
        
        // Update moving objects vertex buffer
        {
            temp_memory mem = begin_temp_memory(arena);
            
            u32 maxVertexCount = 500;
            u32 maxIndexCount = 500;
            u32 vertexCount = 0;
            u32 indexCount = 0;
            
            opengl_vertex *tempVertices = push_array(arena, maxVertexCount, opengl_vertex, 4);
            u32 *tempIndices = push_array(arena, maxIndexCount, u32, 4);
            
            // Player
            v3 offset = cameraTarget.translation;
            v3 size = {1,1,1};
            opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, vertexCount, 1);
            
            memcpy(tempVertices + vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
            memcpy(tempIndices + indexCount, cube.indices, sizeof(u32) * cube.indexCount);
            
            // Register the amount of vertices added
            vertexCount += cube.vertexCount;
            indexCount += cube.indexCount;
            
            opengl_upload_vertexbuffer_data(movingVertexBuffer, vertexCount, tempVertices, indexCount, tempIndices);
            
            end_temp_memory(mem);
        }
        
        camera_update(&camera);
        mat4 view = camera_make_view_matrix(&camera);
        opengl_draw_frame(&os.gl, &view);
    }
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}