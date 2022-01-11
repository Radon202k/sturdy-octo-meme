typedef struct cubes_scene_t
{
    memory_arena arenaPerm;
    memory_arena arenaTran;
    
    GLuint atlasTexture;
    
    f32 *noise2dSeed;
    f32 *noise2d;
    u32 noiseOctave;
    f32 noiseBias;
    
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
cubes_scene_generate_cubes(cubes_scene_t *scene)
{
    memory_arena *arena = &scene->arenaPerm;
    s32 chunkRange = 1;
    
    s32 totalWidth = 5;
    s32 totalHeight = 5;
    
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
                
                for (s32 k = 1;
                     k <= 1;
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
                            
                            f32 height = (f32)((s32)(-10 + 16 * scene->noise2d[(s32)j * 32 + (s32)i]));
                            
                            if (1)
                            {
                                v3 offset = {x,z + height,y};
                                
                                opengl_mesh_indexed cube = opengl_make_cube_mesh_indexed(offset, size, arena, 
                                                                                         scene->cubesVertexBuffer->vertexCount, 
                                                                                         blockType);
                                
                                u8 *ptr = (u8 *)scene->cubesVertexBuffer->vertices;
                                size_t ptrOffset = scene->cubesVertexBuffer->vertexCount * scene->cubesVertexBuffer->vertexSize;
                                memcpy(ptr + ptrOffset, cube.vertices, scene->cubesVertexBuffer->vertexSize * cube.vertexCount);
                                
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
    
    opengl_upload_vertexbuffer_data(scene->cubesVertexBuffer);
}

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
    *scene->cubesVertexBuffer = opengl_make_vertexbuffer(arena, sizeof(f32)*8, 5000000, 500000);
    opengl_vertexbuffer_set_default_inputlayout(scene->cubesVertexBuffer);
    cubes_scene_generate_cubes(scene);
    
    // Second vertex buffer, moving objects
    *scene->movingVertexBuffer = opengl_make_vertexbuffer(arena, sizeof(f32)*8, 10000, 10000);
    opengl_vertexbuffer_set_default_inputlayout(scene->movingVertexBuffer);
    
    // Third vertex buffer, debug text
    *scene->textVertexBuffer = opengl_make_vertexbuffer(arena, sizeof(f32)*8, 1000, 1000);
    opengl_vertexbuffer_set_default_inputlayout(scene->textVertexBuffer);
}

internal void
cubes_scene_load_texture_atlas(cubes_scene_t *scene)
{
    int x,y,n;
    
    char atlasPath[MAX_PATH];
    string_concat(atlasPath, sizeof(atlasPath), os.rootPath, "\\..\\code\\atlas.png");
    
    unsigned char *data = stbi_load(atlasPath, &x, &y, &n, 0);
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
    
    scene->noiseOctave = 2;
    scene->noiseBias = 20.2f;
    
    scene->noise2dSeed = push_array(arena, 32*32, f32, 4);
    scene->noise2d = push_array(arena, 32*32, f32, 4);
    
    perlinlike_noise_seed(32*32, scene->noise2dSeed);
    perlinlike_noise2d(32, 32, scene->noiseOctave, scene->noiseBias, scene->noise2dSeed, scene->noise2d);
    
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
    scene->renderPasses[0] = opengl_make_renderpass(scene->cubesVertexBuffer, renderpass_primitive_triangles, 
                                                    0, scene->atlasTexture, 0, cameraView, perspectiveProj, &textureShader);
    // Moving objects
    scene->renderPasses[1] = opengl_make_renderpass(scene->movingVertexBuffer, renderpass_primitive_triangles,
                                                    0, scene->atlasTexture, 0, cameraView, perspectiveProj, &textureShader);
    // Debug text
    scene->renderPasses[2] = opengl_make_renderpass(scene->textVertexBuffer, renderpass_primitive_triangles,
                                                    0, globalFontTexture, 0, noView, orthographicProj, &textureShader);
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
    
    b32 calculateNoise = 0;
    
    if (os.keyboard.buttons[KEY_SHIFT].down)
    {
        if (os.keyboard.buttons[KEY_O].down)
        {
            if (os.keyboard.buttons[KEY_PLUS].pressed)
            {
                if (scene->noiseOctave < 9)
                {
                    scene->noiseOctave += 1;
                    calculateNoise = 1;
                }
            }
            else if (os.keyboard.buttons[KEY_MINUS].pressed)
            {
                if (scene->noiseOctave > 1)
                {
                    scene->noiseOctave -= 1;
                    calculateNoise = 1;
                }
            }
        }
    }
    
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
    
    if (os.keyboard.buttons[KEY_SHIFT].down)
    {
        if (os.keyboard.buttons[KEY_B].down)
        {
            if (os.keyboard.buttons[KEY_PLUS].pressed)
            {
                scene->noiseBias += 0.2f;
                calculateNoise = 1;
            }
            else if (os.keyboard.buttons[KEY_MINUS].pressed)
            {
                scene->noiseBias -= 0.2f;
                calculateNoise = 1;
            }
        }
    }
    
    if (calculateNoise)
    {
        perlinlike_noise2d(32, 32, scene->noiseOctave, scene->noiseBias, scene->noise2dSeed, scene->noise2d);
        cubes_scene_generate_cubes(scene);
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
                   cube.vertices, scene->movingVertexBuffer->vertexSize * cube.vertexCount);
            
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
        
        stbtt_print(arena, scene->textVertexBuffer, 0, 2*30, "Cubes test scene");
        
        opengl_upload_vertexbuffer_data(scene->textVertexBuffer);
    }
    
    // End temporary memory
    end_temp_memory(mem);
    
    camera_update(&scene->camera);
    
    cubes_scene_update_renderpasses(scene->renderPasses, &scene->camera);
    
    
    opengl_execute_renderpasses(&os.gl, scene->renderPasses, 3);
}