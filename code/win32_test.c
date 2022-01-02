#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_RENDERER
#include "win32/win32_core.h"

GLuint noiseTex;

#include "renderer.h"

WIN32_ENTRY()
{
    memory_arena *arena = win32_init();
    
    renderer_init();
    
    
    // Noise 1d
    u32 noiseOctave = 1;
    f32 noiseBias = 2.0f;
    
    u32 *noisePixels = 0;
    
    f32 *noise1dSeed = push_array(arena, 256, f32, 4);
    f32 *noise1d = push_array(arena, 256, f32, 4);
    
    perlinlike_noise_seed(256, noise1dSeed);
    perlinlike_noise1d(256, noiseOctave, noiseBias, noise1dSeed, noise1d);
    
    f32 *noise2dSeed = push_array(arena, 256*256, f32, 4);
    f32 *noise2d = push_array(arena, 256*256, f32, 4);
    
    perlinlike_noise_seed(256*256, noise2dSeed);
    perlinlike_noise2d(256, 256, noiseOctave, noiseBias, noise2dSeed, noise2d);
    
    
    u32 noiseMode = 1;
    noisePixels = push_array(arena, 256*256, u32, 4);
    for (u32 x = 0;
         x < 256;
         ++x)
    {
        s32 y = (s32)(-(noise1d[x] * 256.0f/2) + 256.0f/2);
        noisePixels[y * 256 + x] = 0xFFFFFF00;
    }
    
    opengl_make_texture(&noiseTex, 256, 256, noisePixels, GL_RGBA, GL_NEAREST);
    
    
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
        
        .distanceFromTarget = 2,
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
        
        b32 calculateNoise = 0; 
        
        if (os.keyboard.buttons[KEY_W].pressed)
        {
            noiseMode = 2;
            
            calculateNoise = 1;
        }
        
        if (os.keyboard.buttons[KEY_S].pressed)
        {
            noiseMode = 1;
            
            calculateNoise = 1;
        }
        
        if (os.keyboard.buttons[KEY_D].pressed)
        {
            noiseBias += 0.2f;
            calculateNoise = 1;
        }
        
        if (os.keyboard.buttons[KEY_A].pressed)
        {
            noiseBias -= 0.2f;
            calculateNoise = 1;
        }
        
        if (os.keyboard.buttons[KEY_CONTROL].pressed)
        {
            if (noiseMode == 1)
            {
                perlinlike_noise_seed(256, noise1dSeed);
            }
            else if (noiseMode == 2)
            {
                perlinlike_noise_seed(256*256, noise2dSeed);
            }
            
            calculateNoise = 1;
        }
        
        if (os.keyboard.buttons[KEY_SPACE].pressed)
        {
            noiseOctave += 1;
            if (noiseOctave == 9)
            {
                noiseOctave = 1;
            }
            calculateNoise = 1;
        }
        
        if (calculateNoise)
        {
            memset(noisePixels, 0, 256*256*4);
            
            if (noiseMode == 1)
            {
                perlinlike_noise1d(256, noiseOctave, noiseBias, noise1dSeed, noise1d);
                for (u32 x = 0;
                     x < 256;
                     ++x)
                {
                    s32 y = (s32)(-(noise1d[x] * 256.0f/2) + 256.0f/2);
                    
                    noisePixels[y * 256 + x] = 0xFFFFFF00;
                    for (s32 f = y; f < 256.0f/2; ++f)
                    {
                        noisePixels[f * 256 + x] = 0xFFFFFF00;
                        
                    }
                }
            }
            else if (noiseMode == 2)
            {
                perlinlike_noise2d(256, 256, noiseOctave, noiseBias, noise2dSeed, noise2d);
                for (u32 y = 0;
                     y < 256;
                     ++y)
                {
                    for (u32 x = 0;
                         x < 256;
                         ++x)
                    {
                        s32 c = (s32)(255.0f * noise2d[y * 256 + x]);
                        
                        noisePixels[y * 256 + x] = (255 << 24) | (c << 16) | (c << 8) | (c << 0);
                    }
                }
            }
            
            opengl_update_texture(&noiseTex, 256, 256, GL_RGBA, noisePixels);
        }
        
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
            
#if 1
            {
                // Player
                v3 offset = cameraTarget.translation;
                v3 size = {1,1,1};
                opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, movingVertexBuffer->vertexCount, 1);
                
                memcpy(movingVertexBuffer->vertices + movingVertexBuffer->vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                memcpy(movingVertexBuffer->indices + movingVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                
                // Register the amount of vertices added
                movingVertexBuffer->vertexCount += cube.vertexCount;
                movingVertexBuffer->indexCount += cube.indexCount;
            }
#else
            
#endif
            
            opengl_upload_vertexbuffer_data(movingVertexBuffer);
        }
        
        // Debug text
        {
            textVertexBuffer->vertexCount = 0;
            textVertexBuffer->indexCount = 0;
            
#if 0
            char fpsBuffer[256] = {0};
            win32_make_label_f32(fpsBuffer, sizeof(fpsBuffer), "Fps", 1.0f / elapsedTime);
            
            stbtt_print(arena, textVertexBuffer, 0, 30, fpsBuffer);
#endif
            
            {
                opengl_mesh_indexed quad = opengl_make_quad_indexed(arena, V2(20,20), V2(480,480), V2(0,0), V2(1,1), textVertexBuffer->vertexCount);
                
                memcpy(textVertexBuffer->vertices + textVertexBuffer->vertexCount, quad.vertices, sizeof(opengl_vertex) * quad.vertexCount);
                memcpy(textVertexBuffer->indices + textVertexBuffer->indexCount, quad.indices, sizeof(u32) * quad.indexCount);
                
                // Register the amount of vertices added
                textVertexBuffer->vertexCount += quad.vertexCount;
                textVertexBuffer->indexCount += quad.indexCount;
                
            }
            
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