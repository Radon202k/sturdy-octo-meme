typedef struct cubes_scene_t
{
    memory_arena arenaPerm;
    memory_arena arenaTran;
    
    GLuint atlasTexture;
    
    transform_t cameraTarget;
    camera_t camera;
    
    v2 mousePos;
    v2 lastMousePos;
    
    typeless_vector vertexBuffers;
    opengl_vertexbuffer *cubesVertexBuffer;
    opengl_vertexbuffer *movingVertexBuffer;
    opengl_vertexbuffer *textVertexBuffer;
    
    opengl_renderpass *renderPasses;
    
} cubes_scene_t;

internal void
cubes_scene_make_vertexbuffers(memory_pool *scenePool)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    memory_arena *arena = &scene->arenaPerm;
    
    // Make vertex buffer array
    scene->vertexBuffers = make_typeless_vector(3, sizeof(opengl_vertexbuffer));
    
    scene->cubesVertexBuffer = &((opengl_vertexbuffer *)scene->vertexBuffers.data)[0];
    scene->movingVertexBuffer = &((opengl_vertexbuffer *)scene->vertexBuffers.data)[1];
    scene->textVertexBuffer = &((opengl_vertexbuffer *)scene->vertexBuffers.data)[2];
    
    // First vertex buffer, static cubes
    // generation code
    {
        *scene->cubesVertexBuffer = opengl_make_vertexbuffer(arena, 5000000, 500000);
        
        s32 chunkRange = 4;
        
        s32 totalWidth = 5;
        s32 totalHeight = 5;
        // f32 *noise = perlinlike_noise2d((f32)totalWidth, (f32)totalHeight, 0.1f, 8);
        
        for (s32 chunkZ = 1;
             chunkZ <= 1;
             ++chunkZ)
        {
            for (s32 chunkY = -chunkRange;
                 chunkY <= chunkRange;
                 ++chunkY)
            {
                for (s32 chunkX = -chunkRange;
                     chunkX <= chunkRange;
                     ++chunkX)
                {
                    s32 range = 4;
                    
                    u32 blockType = rand() % 10 + chunkX;
                    
                    for (s32 k = -range;
                         k <= range;
                         ++k)
                    {
                        for (s32 j = -range;
                             j <= range;
                             ++j)
                        {
                            for (s32 i = -range;
                                 i <= range;
                                 ++i)
                            {
                                f32 scale = 1.0f;
                                v3 size = {scale, scale, scale};
                                
                                f32 x = (f32)(chunkX*(range*2*scale) + (i * size.x));
                                f32 y = (f32)(chunkY*(range*2*scale) + (j * size.y));
                                f32 z = (f32)(chunkZ*(range*2*scale) + (k * size.z));
                                
                                // f32 height = noise[(s32)x * totalWidth + (s32)z];
                                f32 height = -11;
                                
                                if (1)
                                {
                                    v3 offset = {x,z + height,y};
                                    
                                    opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, scene->cubesVertexBuffer->vertexCount, 
                                                                                             blockType);
                                    
                                    memcpy(scene->cubesVertexBuffer->vertices + scene->cubesVertexBuffer->vertexCount, cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
                                    memcpy(scene->cubesVertexBuffer->indices + scene->cubesVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
                                    
                                    // Register the amount of vertices added
                                    scene->cubesVertexBuffer->vertexCount += cube.vertexCount;
                                    scene->cubesVertexBuffer->indexCount += cube.indexCount;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        opengl_upload_vertexbuffer_data_immutable(scene->cubesVertexBuffer);
    }
    
    // Second vertex buffer, moving objects
    *scene->movingVertexBuffer = opengl_make_vertexbuffer(arena, 10000, 10000);
    
    // Third vertex buffer, debug text
    *scene->textVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
}

internal void
cubes_scene_load_texture_atlas(cubes_scene_t *scene)
{
    int x,y,n;
    unsigned char *data = stbi_load("atlas.png", &x, &y, &n, 0);
    if (!data)
    {
        fatal_error("Failed to open image.");
    }
    
    opengl_make_texture(&scene->atlasTexture, x, y, (u32 *)data, GL_RGBA, GL_NEAREST);
    stbi_image_free(data);
}

internal void
cubes_scene_update_renderpasses(opengl_renderpass renderPasses[], camera_t *camera)
{
    renderPasses[0].view = camera_make_view_matrix(camera);
    renderPasses[1].view = camera_make_view_matrix(camera);
    
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    renderPasses[0].proj = mat4_perspective(aspect, 0.6f, 0.1f, 100.0f).forward;
    renderPasses[1].proj = mat4_perspective(aspect, 0.6f, 0.1f, 100.0f).forward;
    renderPasses[2].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
}

internal void
cubes_scene_init(memory_pool *scenePool)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    
    arena_init(&scene->arenaPerm, scenePool->permSize - sizeof(cubes_scene_t),
               (u8 *)scenePool->permBase + sizeof(cubes_scene_t));
    
    memory_arena *arena = &scene->arenaPerm;
    
    cubes_scene_load_texture_atlas(scene);
    
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
    cubes_scene_make_vertexbuffers(scenePool);
    
    // Matrices
    f32 aspect = (f32)os.gl.windowWidth / (f32)os.gl.windowHeight;
    mat4 cameraView = camera_make_view_matrix(&scene->camera);
    mat4 perspectiveProj = mat4_perspective(aspect, 0.3f, 0.1f, 100.0f).forward;
    mat4 noView = mat4_identity(1.0f);
    mat4 orthographicProj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
    
    // Render passes
    
    scene->renderPasses = push_array(arena, 3, opengl_renderpass, 4);
    // Static cubes
    scene->renderPasses[0] = opengl_make_renderpass(scene->cubesVertexBuffer, 0, scene->atlasTexture, cameraView, perspectiveProj);
    // Moving objects
    scene->renderPasses[1] = opengl_make_renderpass(scene->movingVertexBuffer, 0, scene->atlasTexture, cameraView, perspectiveProj);
    // Debug text
    scene->renderPasses[2] = opengl_make_renderpass(scene->textVertexBuffer, 0, globalFontTexture, noView, orthographicProj);
}

internal void
cubes_scene_update(memory_pool *scenePool, f32 elapsedTime)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    
    memory_arena *arena = &scene->arenaPerm;
    
    
    scene->mousePos.x = os.mouse.pos.x;
    scene->mousePos.y = os.mouse.pos.y;
    v2 deltaMousePos = v2_sub(scene->mousePos, scene->lastMousePos);
    scene->lastMousePos = scene->mousePos;
    
    f32 speed = 10;
    
    if (os.keyboard.buttons[KEY_A].down)
    {
        scene->cameraTarget.translation.x -= elapsedTime * speed;
    }
    
    if (os.keyboard.buttons[KEY_D].down)
    {
        scene->cameraTarget.translation.x += elapsedTime * speed;
    }
    
    if (os.keyboard.buttons[KEY_W].down)
    {
        scene->cameraTarget.translation.z -= elapsedTime * speed;
    }
    
    if (os.keyboard.buttons[KEY_S].down)
    {
        scene->cameraTarget.translation.z += elapsedTime * speed;
    }
    
    if (os.mouse.buttons[0].down)
    {
        if (os.keyboard.buttons[KEY_CONTROL].down)
        {
            scene->camera.distanceFromTarget += elapsedTime * deltaMousePos.y;
        }
        else if (os.keyboard.buttons[KEY_SPACE].down)
        {
        }
        else
        {
            scene->camera.pitch += elapsedTime * 0.1f * deltaMousePos.y;
            scene->camera.angleAroundTarget += 0.1f * elapsedTime * deltaMousePos.x;
        }
    }
    
    // Begin temporary memory
    temp_memory mem = begin_temp_memory(arena);
    
    // Update moving objects' vertex buffer
    {
        scene->movingVertexBuffer->vertexCount = 0;
        scene->movingVertexBuffer->indexCount = 0;
        
#if 1
        {
            // Player
            v3 offset = scene->cameraTarget.translation;
            v3 size = {1,1,1};
            opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, scene->movingVertexBuffer->vertexCount, 1);
            
            memcpy(scene->movingVertexBuffer->vertices + scene->movingVertexBuffer->vertexCount, 
                   cube.vertices, sizeof(opengl_vertex) * cube.vertexCount);
            
            memcpy(scene->movingVertexBuffer->indices + scene->movingVertexBuffer->indexCount, 
                   cube.indices, sizeof(u32) * cube.indexCount);
            
            // Register the amount of vertices added
            scene->movingVertexBuffer->vertexCount += cube.vertexCount;
            scene->movingVertexBuffer->indexCount += cube.indexCount;
        }
#else
        
#endif
        
        opengl_upload_vertexbuffer_data(scene->movingVertexBuffer);
    }
    
    // Debug text
    {
        scene->textVertexBuffer->vertexCount = 0;
        scene->textVertexBuffer->indexCount = 0;
        
        char fpsBuffer[256] = {0};
        win32_make_label_f32(fpsBuffer, sizeof(fpsBuffer), "Fps", 1.0f / elapsedTime);
        
        stbtt_print(arena, scene->textVertexBuffer, 0, 30, fpsBuffer);
        
        opengl_upload_vertexbuffer_data(scene->textVertexBuffer);
    }
    
    // End temporary memory
    end_temp_memory(mem);
    
    camera_update(&scene->camera);
    
    cubes_scene_update_renderpasses(scene->renderPasses, &scene->camera);
    
    
    opengl_draw_frame(&os.gl, scene->renderPasses, 3);
}