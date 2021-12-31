#define _CRT_SECURE_NO_WARNINGS

#define OPENGL_RENDERER
#include "win32/win32_core.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void my_stbtt_initfont(memory_arena *arena)
{
    fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
    
    u32 *output = push_array(arena, 512*512, u32, 4);
    
    for (u32 y = 0;
         y < 512;
         ++y)
    {
        for (u32 x = 0;
             x < 512;
             ++x)
        {
            u8 alpha = temp_bitmap[y * 512 + x];
            
            if (alpha == 255)
            {
                int breakHere = 2;
            }
            
            output[y * 512 + x] = (alpha << 24) | (alpha << 16) | (alpha << 8) | (alpha << 0);
        }
    }
    
    opengl_make_texture(&ftex, 512, 512, output, GL_RGBA, GL_LINEAR);
}

void my_stbtt_print(memory_arena *arena, opengl_vertexbuffer *b, float x, float y, char *text)
{
    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
            
            opengl_mesh_indexed quad = opengl_make_quad_indexed(arena, V2(q.x0,q.y0), V2(q.x1,q.y1), 
                                                                V2(q.s0,q.t0), V2(q.s1,q.t1), b->vertexCount);
            
            memcpy(b->vertices + b->vertexCount, quad.vertices, sizeof(opengl_vertex) * quad.vertexCount);
            memcpy(b->indices + b->indexCount, quad.indices, sizeof(u32) * quad.indexCount);
            
            b->vertexCount += quad.vertexCount;
            b->indexCount += quad.indexCount;
        }
        ++text;
    }
}

WIN32_ENTRY()
{
    win32_init();
    
    memory_arena *arena = &os.mainArena;
    
    opengl_init(&os.gl, 0, 0, 500, 500, "Minecraft clone", arena);
    
    my_stbtt_initfont(arena);
    
    int x,y,n;
    unsigned char *data = stbi_load("atlas.png", &x, &y, &n, 0);
    if (!data)
    {
        fatal_error("Failed to open image.");
    }
    
    opengl_make_texture(&os.gl.texture, x, y, (u32 *)data, GL_RGBA, GL_NEAREST);
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
        
        .distanceFromTarget = 10,
        .angleAroundTarget = 0,
    };
    
    v2 mousePos = {.x = os.mouse.pos.x, .y = os.mouse.pos.y};
    
    v2 lastMousePos = mousePos;
    
    // Make vertex buffer array
    os.gl.vertexBuffers = make_typeless_vector(3, sizeof(opengl_vertexbuffer));
    
    opengl_vertexbuffer *cubesVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[0];
    opengl_vertexbuffer *movingVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[1];
    opengl_vertexbuffer *textVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[2];
    
    /* 

NOTE:

- First vertex buffer for cubes
     - Second for moving objects

*/
    
    // First vertex buffer, static cubes
    // generation code
    {
        *cubesVertexBuffer = opengl_make_vertexbuffer(arena, 500000, 50000);
        
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
                opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, cubesVertexBuffer->vertexCount, 0);
                
                memcpy(cubesVertexBuffer->vertices + cubesVertexBuffer->vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                memcpy(cubesVertexBuffer->indices + cubesVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                
                // Register the amount of vertices added
                cubesVertexBuffer->vertexCount += cube.vertexCount;
                cubesVertexBuffer->indexCount += cube.indexCount;
            }
        }
        
        opengl_upload_vertexbuffer_data_immutable(cubesVertexBuffer);
    }
    
    // Second vertex buffer, moving objects
    *movingVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
    
    // Third vertex buffer, debug text
    *textVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
    
    opengl_renderpass renderPasses[3] = {0};
    
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    
    // Static cubes render pass
    renderPasses[0].buffer = cubesVertexBuffer;
    renderPasses[0].textureUnit = 0;
    renderPasses[0].textureHandle = os.gl.texture;
    renderPasses[0].view = camera_make_view_matrix(&camera);
    renderPasses[0].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    
    // Moving objects render pass
    renderPasses[1].buffer = movingVertexBuffer;
    renderPasses[1].textureUnit = 0;
    renderPasses[1].textureHandle = os.gl.texture;
    renderPasses[1].view = camera_make_view_matrix(&camera);
    renderPasses[1].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    
    // Font text render pass
    renderPasses[2].buffer = textVertexBuffer;
    renderPasses[2].textureUnit = 0;
    renderPasses[2].textureHandle = ftex;
    renderPasses[2].view = mat4_identity(1.0f);
    renderPasses[2].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
    
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
        
        // Update moving objects vertex buffer
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
            
            my_stbtt_print(arena, textVertexBuffer, 0, 30, fpsBuffer);
            
            opengl_upload_vertexbuffer_data(textVertexBuffer);
        }
        
        // End temporary memory
        end_temp_memory(mem);
        
        
        camera_update(&camera);
        
        renderPasses[0].view = camera_make_view_matrix(&camera);
        renderPasses[1].view = camera_make_view_matrix(&camera);
        
        aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
        renderPasses[0].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
        renderPasses[1].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
        renderPasses[2].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
        
        opengl_draw_frame(&os.gl, renderPasses, 3);
    }
    
    // Renderer cleanup
    opengl_cleanup(&os.gl);
    
    return 0;
}