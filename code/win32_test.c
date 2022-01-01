#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_RENDERER
#include "win32/win32_core.h"

#include "renderer.h"

WIN32_ENTRY()
{
    memory_arena *arena = win32_init();
    
    renderer_init();
    
    // "Player"
    transform_t cameraTarget = transform_get_default();
    cameraTarget.translation.y = 2;
    cameraTarget.rotation.x = 0;
    
    // 3rd person camera following player
    camera_t camera = 
    {
        .pos = {0,0,0},
        .pitch = 0,
        .yaw = 0,
        .roll = 0,
        
        .targetTransform = &cameraTarget,
        
        .distanceFromTarget = 10,
        .angleAroundTarget = 0,
    };
    
    v2 mousePos = {.x = os.mouse.pos.x, .y = os.mouse.pos.y};
    v2 lastMousePos = mousePos;
    
    // Vertex buffers
    make_vertexbuffers(arena);
    opengl_vertexbuffer *cubesVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[0];
    opengl_vertexbuffer *movingVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[1];
    opengl_vertexbuffer *textVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[2];
    
    // Matrices
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    mat4 cameraView = camera_make_view_matrix(&camera);
    mat4 perspectiveProj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    mat4 noView = mat4_identity(1.0f);
    mat4 orthographicProj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
    
    // Render passes
    opengl_renderpass *renderPasses = renderer_make_renderpasses(arena, 
                                                                 cubesVertexBuffer, movingVertexBuffer, 
                                                                 textVertexBuffer, &cameraView, &noView,
                                                                 &perspectiveProj, &orthographicProj);
    
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
        
        f32 speed = 10;
        
        if (os.keyboard.buttons[KEY_A].down)
        {
            cameraTarget.translation.x -= elapsedTime * speed;
        }
        
        if (os.keyboard.buttons[KEY_D].down)
        {
            cameraTarget.translation.x += elapsedTime * speed;
        }
        
        if (os.keyboard.buttons[KEY_W].down)
        {
            cameraTarget.translation.z -= elapsedTime * speed;
        }
        
        if (os.keyboard.buttons[KEY_S].down)
        {
            cameraTarget.translation.z += elapsedTime * speed;
        }
        
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
        
        // Begin temporary memory
        temp_memory mem = begin_temp_memory(arena);
        
        // Update moving objects' vertex buffer
        {
            movingVertexBuffer->vertexCount = 0;
            movingVertexBuffer->indexCount = 0;
            
            // Player
            v3 offset = cameraTarget.translation;
            v3 size = {1,1,1};
            opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, movingVertexBuffer->vertexCount, 1);
            
            memcpy(movingVertexBuffer->vertices + movingVertexBuffer->vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
            memcpy(movingVertexBuffer->indices + movingVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
            
            // Register the amount of vertices added
            movingVertexBuffer->vertexCount += cube.vertexCount;
            movingVertexBuffer->indexCount += cube.indexCount;
            
            opengl_upload_vertexbuffer_data(movingVertexBuffer);
        }
        
        // Debug text
        {
            textVertexBuffer->vertexCount = 0;
            textVertexBuffer->indexCount = 0;
            
            char fpsBuffer[256] = {0};
            win32_make_label_f32(fpsBuffer, sizeof(fpsBuffer), "Fps", 1.0f / elapsedTime);
            
            stbtt_print(arena, textVertexBuffer, 0, 30, fpsBuffer);
            
            opengl_upload_vertexbuffer_data(textVertexBuffer);
        }
        
        // End temporary memory
        end_temp_memory(mem);
        
        camera_update(&camera);
        
        update_renderpasses(renderPasses, &camera);
        
        opengl_draw_frame(&os.gl, renderPasses, 3);
    }
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}