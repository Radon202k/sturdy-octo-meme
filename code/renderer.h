/* date = January 1st 2022 11:28 am */

#ifndef RENDERER_H
#define RENDERER_H

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "stb_text.h"

#include <time.h>
time_t t;

internal void
make_vertexbuffers(memory_arena *arena)
{
    // Make vertex buffer array
    os.gl.vertexBuffers = make_typeless_vector(3, sizeof(opengl_vertexbuffer));
    
    opengl_vertexbuffer *cubesVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[0];
    opengl_vertexbuffer *movingVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[1];
    opengl_vertexbuffer *textVertexBuffer = &((opengl_vertexbuffer *)os.gl.vertexBuffers.data)[2];
    
    // First vertex buffer, static cubes
    // generation code
    {
        *cubesVertexBuffer = opengl_make_vertexbuffer(arena, 5000000, 500000);
        
        s32 chunkRange = 2;
        for (s32 chunkZ = -chunkRange;
             chunkZ <= chunkRange;
             ++chunkZ)
        {
            for (s32 chunkX = -chunkRange;
                 chunkX <= chunkRange;
                 ++chunkX)
            {
                s32 range = 16;
                
                u32 blockType = rand() % 10 + chunkX;
                
                for (s32 j = -range;
                     j <= range;
                     ++j)
                {
                    for (s32 i = -range;
                         i <= range;
                         ++i)
                    {
                        f32 scale = 0.5f;
                        v3 size = {scale, scale, scale};
                        
                        s32 height = 10 + (s32)(-20.0f * (0.707f + perlin2d((f32)i, (f32)j, 0.02f, 4)) / (0.707f*2));
                        
                        v3 offset = 
                        {
                            ((f32)chunkX*(range)*2*scale + ((f32)i * size.x)), 
                            ((f32)height * size.y), 
                            ((f32)chunkZ*(range)*2*scale + ((f32)j * size.z))
                        };
                        
                        opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, cubesVertexBuffer->vertexCount, 
                                                                                 blockType);
                        
                        memcpy(cubesVertexBuffer->vertices + cubesVertexBuffer->vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                        memcpy(cubesVertexBuffer->indices + cubesVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                        
                        // Register the amount of vertices added
                        cubesVertexBuffer->vertexCount += cube.vertexCount;
                        cubesVertexBuffer->indexCount += cube.indexCount;
                    }
                }
            }
        }
        
        opengl_upload_vertexbuffer_data_immutable(cubesVertexBuffer);
    }
    
    // Second vertex buffer, moving objects
    *movingVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
    
    // Third vertex buffer, debug text
    *textVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
}

internal void
load_texture_atlas()
{
    int x,y,n;
    unsigned char *data = stbi_load("atlas.png", &x, &y, &n, 0);
    if (!data)
    {
        fatal_error("Failed to open image.");
    }
    
    opengl_make_texture(&os.gl.texture, x, y, (u32 *)data, GL_RGBA, GL_NEAREST);
    stbi_image_free(data);
}

internal void
update_renderpasses(opengl_renderpass renderPasses[], camera_t *camera)
{
    renderPasses[0].view = camera_make_view_matrix(camera);
    renderPasses[1].view = camera_make_view_matrix(camera);
    
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    renderPasses[0].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    renderPasses[1].proj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    renderPasses[2].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
}

internal void
renderer_init(void)
{
    opengl_init(&os.gl, 0, 0, 500, 500, "Minecraft clone", &os.mainArena);
    
    stbtt_initfont(&os.mainArena);
    
    os.window_is_open = 1;
    
    // Load texture atlas
    load_texture_atlas();
    
    srand((unsigned) time(&t));
}

internal opengl_renderpass *
renderer_make_renderpasses(memory_arena *arena,
                           opengl_vertexbuffer *cubesVertexBuffer,
                           opengl_vertexbuffer *movingVertexBuffer,
                           opengl_vertexbuffer *textVertexBuffer,
                           mat4 *cameraView,
                           mat4 *noView,
                           mat4 *perspectiveProj,
                           mat4 *orthographicProj)
{
    opengl_renderpass *renderPasses = push_array(arena, 3, opengl_renderpass, 4);
    // Static cubes
    renderPasses[0] = opengl_make_renderpass(cubesVertexBuffer, 0, os.gl.texture, *cameraView, *perspectiveProj);
    // Moving objects
    renderPasses[1] = opengl_make_renderpass(movingVertexBuffer, 0, os.gl.texture, *cameraView, *perspectiveProj);
    // Debug text
    renderPasses[2] = opengl_make_renderpass(textVertexBuffer, 0, ftex, *noView, *orthographicProj);
    return renderPasses;
}

#endif //RENDERER_H
