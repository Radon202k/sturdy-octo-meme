typedef struct perlin_scene_t
{
    memory_arena arenaPerm;
    memory_arena arenaTran;
    
    u32 noiseOctave;
    f32 noiseBias;
    
    u32 *noisePixels;
    
    f32 *noise1dSeed;
    f32 *noise1d;
    
    f32 *noise2dSeed;
    f32 *noise2d;
    
    u32 noiseMode;
    
    GLuint noiseTexture;
    
    transform_t cameraTarget;
    camera_t camera;
    
    v2 mousePos;
    v2 lastMousePos;
    
    typeless_vector vertexBuffers;
    opengl_vertexbuffer *canvas2dVertexBuffer;
    
    opengl_renderpass *renderPasses;
    
} perlin_scene_t;

internal void
perlin_scene_make_vertexbuffers(memory_pool *scenePool)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    memory_arena *arena = &scene->arenaPerm;
    
    // Make vertex buffer array
    scene->vertexBuffers = make_typeless_vector(1, sizeof(opengl_vertexbuffer));
    
    scene->canvas2dVertexBuffer = &((opengl_vertexbuffer *)scene->vertexBuffers.data)[0];
    
    *scene->canvas2dVertexBuffer = opengl_make_vertexbuffer(arena, 1000, 1000);
    opengl_vertexbuffer_set_default_inputlayout(scene->canvas2dVertexBuffer);
}

internal void
perlin_update_renderpasses(opengl_renderpass renderPasses[], camera_t *camera)
{
    renderPasses[0].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
}

internal void
perlin_scene_init(memory_pool *scenePool)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    
    arena_init(&scene->arenaPerm, scenePool->permSize - sizeof(perlin_scene_t),
               (u8 *)scenePool->permBase + sizeof(perlin_scene_t));
    
    memory_arena *arena = &scene->arenaPerm;
    
    // Noise 1d
    scene->noiseOctave = 1;
    scene->noiseBias = 2.0f;
    
    scene->noisePixels = 0;
    
    scene->noise1dSeed = push_array(arena, 256, f32, 4);
    scene->noise1d = push_array(arena, 256, f32, 4);
    
    perlinlike_noise_seed(256, scene->noise1dSeed);
    perlinlike_noise1d(256, scene->noiseOctave, scene->noiseBias, scene->noise1dSeed, scene->noise1d);
    
    scene->noise2dSeed = push_array(arena, 256*256, f32, 4);
    scene->noise2d = push_array(arena, 256*256, f32, 4);
    
    perlinlike_noise_seed(256*256, scene->noise2dSeed);
    perlinlike_noise2d(256, 256, scene->noiseOctave, scene->noiseBias, scene->noise2dSeed, scene->noise2d);
    
    scene->noiseMode = 1;
    scene->noisePixels = push_array(arena, 256*256, u32, 4);
    for (u32 x = 0;
         x < 256;
         ++x)
    {
        s32 y = (s32)(-(scene->noise1d[x] * 256.0f/2) + 256.0f/2);
        scene->noisePixels[y * 256 + x] = 0xFFFFFF00;
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
    
    scene->renderPasses = push_array(arena, 1, opengl_renderpass, 4);
    scene->renderPasses[0] = opengl_make_renderpass(scene->canvas2dVertexBuffer, renderpass_primitive_triangles,
                                                    0, scene->noiseTexture, cameraView, perspectiveProj, mainPipeline);
}

internal void
perlin_scene_update(memory_pool *scenePool, f32 elapsedTime)
{
    perlin_scene_t *scene = (perlin_scene_t *)scenePool->permBase;
    
    memory_arena *arena = &scene->arenaPerm;
    
    
    scene->mousePos.x = os.mouse.pos.x;
    scene->mousePos.y = os.mouse.pos.y;
    v2 deltaMousePos = v2_sub(scene->mousePos, scene->lastMousePos);
    scene->lastMousePos = scene->mousePos;
    
    f32 speed = 10;
    
    b32 calculateNoise = 0; 
    
    if (os.keyboard.buttons[KEY_1].pressed)
    {
        scene->noiseMode = 1;
        calculateNoise = 1;
    }
    
    if (os.keyboard.buttons[KEY_2].pressed)
    {
        scene->noiseMode = 2;
        calculateNoise = 1;
    }
    
    if (os.keyboard.buttons[KEY_PLUS].pressed)
    {
        scene->noiseBias += 0.2f;
        calculateNoise = 1;
    }
    
    if (os.keyboard.buttons[KEY_MINUS].pressed)
    {
        scene->noiseBias -= 0.2f;
        calculateNoise = 1;
    }
    
    if (os.keyboard.buttons[KEY_SHIFT].down)
    {
        if (os.keyboard.buttons[KEY_PLUS].pressed)
        {
            if (scene->noiseOctave < 9)
            {
                scene->noiseOctave += 1;
            }
        }
        else if (os.keyboard.buttons[KEY_MINUS].pressed)
        {
            if (scene->noiseOctave > 1)
            {
                scene->noiseOctave -= 1;
            }
        }
        
        calculateNoise = 1;
    }
    
    if (os.keyboard.buttons[KEY_CONTROL].down && os.keyboard.buttons[KEY_A].pressed)
    {
        if (scene->noiseMode == 1)
        {
            perlinlike_noise_seed(256, scene->noise1dSeed);
        }
        else if (scene->noiseMode == 2)
        {
            perlinlike_noise_seed(256*256, scene->noise2dSeed);
        }
        
        calculateNoise = 1;
    }
    
    if (calculateNoise)
    {
        memset(scene->noisePixels, 0, 256*256*4);
        
        if (scene->noiseMode == 1)
        {
            perlinlike_noise1d(256, scene->noiseOctave, scene->noiseBias, scene->noise1dSeed, scene->noise1d);
            for (u32 x = 0;
                 x < 256;
                 ++x)
            {
                s32 y = (s32)(-(scene->noise1d[x] * 256.0f/2) + 256.0f/2);
                
                scene->noisePixels[y * 256 + x] = 0xFFFFFF00;
                for (s32 f = y; f < 256.0f/2; ++f)
                {
                    scene->noisePixels[f * 256 + x] = 0xFFFFFF00;
                    
                }
            }
        }
        else if (scene->noiseMode == 2)
        {
            perlinlike_noise2d(256, 256, scene->noiseOctave, scene->noiseBias, scene->noise2dSeed, scene->noise2d);
            for (u32 y = 0;
                 y < 256;
                 ++y)
            {
                for (u32 x = 0;
                     x < 256;
                     ++x)
                {
                    s32 c = (s32)(255.0f * scene->noise2d[y * 256 + x]);
                    
                    scene->noisePixels[y * 256 + x] = (255 << 24) | (c << 16) | (c << 8) | (c << 0);
                }
            }
        }
        
        opengl_update_texture(&scene->noiseTexture, 256, 256, GL_RGBA, scene->noisePixels);
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
    
    scene->canvas2dVertexBuffer->vertexCount = 0;
    scene->canvas2dVertexBuffer->indexCount = 0;
    
    opengl_mesh_indexed quad = opengl_make_quad_indexed(arena, V2(30,30), V2(470,470), V2(0,0), V2(1,1), scene->canvas2dVertexBuffer->vertexCount);
    
    memcpy(scene->canvas2dVertexBuffer->vertices + scene->canvas2dVertexBuffer->vertexCount, quad.vertices, sizeof(opengl_vertex) * quad.vertexCount);
    memcpy(scene->canvas2dVertexBuffer->indices + scene->canvas2dVertexBuffer->indexCount, quad.indices, sizeof(u32) * quad.indexCount);
    
    // Register the amount of vertices added
    scene->canvas2dVertexBuffer->vertexCount += quad.vertexCount;
    scene->canvas2dVertexBuffer->indexCount += quad.indexCount;
    
    opengl_upload_vertexbuffer_data(scene->canvas2dVertexBuffer);
    
    // End temporary memory
    end_temp_memory(mem);
    
    camera_update(&scene->camera);
    
    perlin_update_renderpasses(scene->renderPasses, &scene->camera);
    
    opengl_execute_renderpasses(&os.gl, scene->renderPasses, 1);
}