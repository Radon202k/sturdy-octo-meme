#include "open-simplex-noise.h"
#include "open-simplex-noise.c"

typedef struct perlin_scene_t
{  
    memory_arena_t arenaPerm;
    memory_arena_t arenaTran;
    
    u32 *noisePixels;
    
    opensimplex_noise_t noise;
    
    struct osn_context *ctx;
    
    GLuint noiseTexture;
    
    transform_t cameraTarget;
    camera_t camera;
    
    v2 mousePos;
    v2 lastMousePos;
    
    typeless_vector_t vertexBuffers;
    gl_vbuffer_t *canvas2dVertexBuffer;
    
    gl_renderpass_t *renderPasses;
    
} perlin_scene_t;

internal void
perlin_scene_make_vertexbuffers(memory_pool_t *scenePool)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    memory_arena_t *arena = &scene->arenaPerm;
    
    // Make vertex buffer array
    scene->vertexBuffers = make_typeless_vector(1, sizeof(gl_vbuffer_t));
    
    scene->canvas2dVertexBuffer = &((gl_vbuffer_t *)scene->vertexBuffers.data)[0];
    
    *scene->canvas2dVertexBuffer = opengl_make_vbuffer(arena, sizeof(f32)*8, megabytes(2));
    opengl_vbuffer_set_default_inputlayout(scene->canvas2dVertexBuffer);
}

internal void
perlin_update_renderpasses(gl_renderpass_t renderPasses[], camera_t *camera)
{
    renderPasses[0].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
}

internal void
perlin_scene_init(memory_pool_t *scenePool)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    
    arena_init(&scene->arenaPerm, scenePool->permSize - sizeof(perlin_scene_t),
               (u8 *)scenePool->permBase + sizeof(perlin_scene_t));
    
    memory_arena_t *arena = &scene->arenaPerm;
    
#if 0
    scene->noise = make_opensimplex_noise();
    
    
    
#endif 
    
    open_simplex_noise(0, &scene->ctx);
    
    scene->noisePixels = push_size(arena, 256*256, 4);
    
    for (u32 y = 0;
         y < 256;
         ++y)
    {
        for (u32 x = 0;
             x < 256;
             ++x)
        {
            f64 noiseValue = open_simplex_noise2(scene->ctx, (double) x/32, (double) y/32);
            
            
            u32 color = (u32)(255.0 * ((0.5 * noiseValue) + 0.5));
            scene->noisePixels[y * 256 + x] = (255 << 24) | (color << 16) | (color << 8) | (color << 0);
        }
    }
    
    opengl_make_texture(&scene->noiseTexture, 256, 256, scene->noisePixels, GL_RGBA, GL_NEAREST);
    
    // "Player"
    scene->cameraTarget = transform_get_default();
    scene->cameraTarget.translation.y = 2;
    scene->cameraTarget.rotation.x = 0;
    
    // 3rd person camera following player
    scene->camera.targetTransform = &scene->cameraTarget;
    scene->camera.distanceFromTarget = 2,
    
    scene->mousePos.x = os.mouse.pos.x;
    scene->mousePos.y = os.mouse.pos.y;
    scene->lastMousePos = scene->mousePos;
    
    // Vertex buffers
    perlin_scene_make_vertexbuffers(scenePool);
    
    // Matrices
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    mat4 cameraView = camera_make_view_matrix(&scene->camera);
    mat4 perspectiveProj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    mat4 noView = mat4_identity(1.0f);
    mat4 orthographicProj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
    
    // Render passes
    
    scene->renderPasses = push_array(arena, 1, gl_renderpass_t, 4);
    scene->renderPasses[0] = opengl_make_renderpass(scene->canvas2dVertexBuffer, gl_primitive_triangles,
                                                    0, scene->noiseTexture, 0, cameraView, perspectiveProj, &textureShader);
}

internal void
perlin_scene_update(memory_pool_t *scenePool, f32 elapsedTime)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    
    memory_arena_t *arena = &scene->arenaPerm;
    
    scene->mousePos.x = os.mouse.pos.x;
    scene->mousePos.y = os.mouse.pos.y;
    v2 deltaMousePos = v2_sub(scene->mousePos, scene->lastMousePos);
    scene->lastMousePos = scene->mousePos;
    
    // Begin temporary memory
    temp_memory_t mem = begin_temp_memory(arena);
    
    scene->canvas2dVertexBuffer->vertexCount = 0;
    scene->canvas2dVertexBuffer->indexCount = 0;
    
    gl_mesh_t quad = opengl_make_quad(arena, V2(30,30), V2(470,470), V2(0,0), V2(1,1), scene->canvas2dVertexBuffer->vertexCount);
    
    memcpy(scene->canvas2dVertexBuffer->vertices + scene->canvas2dVertexBuffer->vertexCount, quad.vertices, scene->canvas2dVertexBuffer->vertexSize * quad.vertexCount);
    memcpy(scene->canvas2dVertexBuffer->indices + scene->canvas2dVertexBuffer->indexCount, quad.indices, sizeof(u32) * quad.indexCount);
    
    // Register the amount of vertices added
    scene->canvas2dVertexBuffer->vertexCount += quad.vertexCount;
    scene->canvas2dVertexBuffer->indexCount += quad.indexCount;
    
    opengl_upload_vbuffer_data(scene->canvas2dVertexBuffer);
    
    // End temporary memory
    end_temp_memory(mem);
    
    camera_update(&scene->camera);
    
    perlin_update_renderpasses(scene->renderPasses, &scene->camera);
    
    opengl_execute_renderpasses(&os.gl, scene->renderPasses, 1);
}