/*

Try:
- Make noise generate surface only? everything below is solid?
start generation at top of chunk then generate everything below it?

- Cast rays from camera to know which faces to draw?

- Generate vertex buffer every frame, only draw visible faces
ask per cube if there are cubes around? that doesnt seem so reliable,
but maybe because of the geometry is usually filled? what about caves?
should chunks be rendered from bottom of the world up or should it be
cut off based on player's Y position?

- Time frames, keep buffer with 32 frames or so and add ability to see
 which functions are stalling so we can debug frame drops...

- how much memory is OK to use for vertex buffer to draw cubes? 32 mb? 
256 mb? 2 gb? and index buffer?

- direction sun light (simple phong),  shadows??

*/

typedef struct cubes_scene_t
{
    memory_arena_t arenaPerm;
    memory_arena_t arenaTemp;
    
    GLuint atlasTexture;
    
    struct osn_context *noiseContext;
    u32 noiseHeight;
    f32 noiseBias;
    
    transform_t cameraTarget;
    camera_t camera;
    
    v2 mousePos;
    v2 lastMousePos;
    
    typeless_vector_t vertexBuffers;
    gl_vbuffer_t *cubesVertexBuffer;
    gl_vbuffer_t *linesVertexBuffer;
    gl_vbuffer_t *textVertexBuffer;
    
    gl_renderpass_t *renderPasses;
    
    
} cubes_scene_t;

internal void
generate_cube(cubes_scene_t *scene, s32 x, s32 y, s32 z, u32 cubeDim,
              s32 chunkX, s32 chunkY, s32 chunkZ, u32 chunkDim,
              u32 blockType)
{
    f64 noiseX = (f64)(chunkX * (s32)chunkDim + x);
    f64 noiseY = (f64)(chunkZ * (s32)chunkDim + z);
    
    f64 noiseValue = open_simplex_noise2(scene->noiseContext, (double) noiseX / scene->noiseBias,
                                         (double) noiseY / scene->noiseBias);
    
    u32 height = (u32)(scene->noiseHeight * ((0.5 * noiseValue) + 0.5));
    
    if (1)
    {
        v3 offset = 
        {
            (f32)(chunkX * (s32)chunkDim) + (f32)x,
            (f32)(chunkY * (s32)chunkDim) + (f32)y + (f32)height,
            (f32)(chunkZ * (s32)chunkDim) + (f32)z
        };
        
        gl_mesh_t cube = opengl_make_cube_mesh(offset, cubeDim, &scene->arenaTemp, 
                                               scene->cubesVertexBuffer->vertexCount, 
                                               blockType);
        
        // Copy generated data to cubes' vertex buffer
        u8 *ptr = (u8 *)scene->cubesVertexBuffer->vertices;
        size_t ptrOffset = scene->cubesVertexBuffer->vertexCount * scene->cubesVertexBuffer->vertexSize;
        memcpy(ptr + ptrOffset, cube.vertices, scene->cubesVertexBuffer->vertexSize * cube.vertexCount);
        
        memcpy(scene->cubesVertexBuffer->indices + scene->cubesVertexBuffer->indexCount, cube.indices, sizeof(u32) * cube.indexCount);
        
        // Register the amount of vertices added
        scene->cubesVertexBuffer->vertexCount += cube.vertexCount;
        scene->cubesVertexBuffer->indexCount += cube.indexCount;
    }
}

internal void
generate_chunk(cubes_scene_t *scene, v3 chunkDim, s32 chunkX, s32 chunkY, s32 chunkZ)
{
    s32 range2d = (s32)(chunkDim.x/2);
    
    // u32 blockType = rand() % 10 + chunkX;
    u32 blockType = 0;
    
    temp_memory_t mem = begin_temp_memory(&scene->arenaTemp);
    
    u32 chunkDimSq = (u32)(chunkDim.x*chunkDim.x);
    
    for (u32 j = 0;
         j <= (u32)(chunkDim.y);
         ++j)
    {
        for (u32 i = 0;
             i <= (u32)(chunkDim.x);
             ++i)
        {
            u32 cubeDim = 1;
            
            u32 x = (i * cubeDim);
            u32 y = 0;
            u32 z = (j * cubeDim);
            
            generate_cube(scene, x, y, z, cubeDim,
                          chunkX, chunkY, chunkZ, (s32)chunkDim.x,
                          blockType);
        }
    }
    
    
    end_temp_memory(mem);
}

internal void
cubes_scene_generate_cubes(cubes_scene_t *scene)
{
    // Reset line vertex buffer
    scene->linesVertexBuffer->vertexCount = 0;
    scene->linesVertexBuffer->indexCount = 0;
    
    // Draw global axes
    draw_line(scene->linesVertexBuffer,
              V3(0,0,0), V3(100,0,0), V4(1,0,0,1));
    
    draw_line(scene->linesVertexBuffer,
              V3(0,0,0), V3(0,100,0), V4(0,0,1,1));
    
    draw_line(scene->linesVertexBuffer,
              V3(0,0,0), V3(0,0,100), V4(0,1,0,1));
    
    
    memory_arena_t *arena = &scene->arenaPerm;
    s32 chunkRange = 2;
    
    s32 totalWidth = 5;
    s32 totalHeight = 5;
    
    v3 chunkDim = V3(16,20,16);
    
    for (s32 chunkZ = -chunkRange;
         chunkZ <= chunkRange;
         ++chunkZ)
    {
        for (s32 chunkY = 0;
             chunkY < 1;
             ++chunkY)
        {
            for (s32 chunkX = -chunkRange;
                 chunkX <= chunkRange;
                 ++chunkX)
            {
                f32 chunkMinX = (f32)chunkX * chunkDim.x;
                f32 chunkMinY = (f32)chunkY * chunkDim.y;
                f32 chunkMinZ = (f32)chunkZ * chunkDim.z;
                
                f32 chunkMaxX = (f32)(chunkMinX + chunkDim.x);
                f32 chunkMaxY = (f32)(chunkMinY + chunkDim.y);
                f32 chunkMaxZ = (f32)(chunkMinZ + chunkDim.z);
                
                v3 chunkMin = V3((f32)((s32)chunkMinX),(f32)((s32)chunkMinY),(f32)((s32)chunkMinZ));
                v3 chunkMax = V3((f32)((s32)chunkMaxX),(f32)((s32)chunkMaxY),(f32)((s32)chunkMaxZ));
                
                v3 chunkSize = v3_sub(chunkMax, chunkMin);
                v3 chunkCenter = v3_add(chunkMin, v3_mul(chunkSize, 0.5f));
                
                draw_bounding_box(scene->linesVertexBuffer, 
                                  rect3_center_dim(chunkCenter,chunkSize));
                
                
                generate_chunk(scene, chunkDim, chunkX, chunkY, chunkZ);
            }
        }
    }
    
    opengl_upload_vbuffer_data(scene->cubesVertexBuffer);
    opengl_upload_vbuffer_data(scene->linesVertexBuffer);
}

internal void
cubes_scene_make_vertexbuffers(memory_pool_t *scenePool)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    memory_arena_t *arena = &scene->arenaPerm;
    
    // Make vertex buffer array
    scene->vertexBuffers = make_typeless_vector(4, sizeof(gl_vbuffer_t));
    
    scene->cubesVertexBuffer = &((gl_vbuffer_t *)scene->vertexBuffers.data)[0];
    scene->linesVertexBuffer = &((gl_vbuffer_t *)scene->vertexBuffers.data)[1];
    scene->textVertexBuffer = &((gl_vbuffer_t *)scene->vertexBuffers.data)[2];
    
    // Lines
    *scene->linesVertexBuffer = opengl_make_vbuffer(arena, sizeof(f32)*3+sizeof(u32), megabytes(2));
    opengl_vbuffer_set_inputlayout(scene->linesVertexBuffer, 0, GL_FLOAT, 3, 0);
    opengl_vbuffer_set_inputlayout(scene->linesVertexBuffer, 1, GL_UNSIGNED_BYTE, 4, sizeof(f32)*3);
    
    // First vertex buffer, static cubes
    *scene->cubesVertexBuffer = opengl_make_vbuffer(arena, sizeof(f32)*8, megabytes(32));
    opengl_vbuffer_set_default_inputlayout(scene->cubesVertexBuffer);
    cubes_scene_generate_cubes(scene);
    
    // Third vertex buffer, debug text
    *scene->textVertexBuffer = opengl_make_vbuffer(arena, sizeof(f32)*8, megabytes(2));
    opengl_vbuffer_set_default_inputlayout(scene->textVertexBuffer);
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
cubes_scene_update_renderpasses(gl_renderpass_t renderPasses[], camera_t *camera)
{
    renderPasses[0].view = camera_make_view_matrix(camera);
    renderPasses[1].view = camera_make_view_matrix(camera);
    
    renderPasses[0].proj = mat4_infinite_z_perspective((f32)os.gl.windowWidth, (f32)os.gl.windowHeight, 15).forward;
    renderPasses[1].proj = mat4_infinite_z_perspective((f32)os.gl.windowWidth, (f32)os.gl.windowHeight, 15).forward;
    renderPasses[2].proj = mat4_orthographic((f32)os.gl.windowWidth, (f32)os.gl.windowHeight).forward;
}

internal void
cubes_scene_init(memory_pool_t *scenePool)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    
    arena_init(&scene->arenaPerm, scenePool->permSize - sizeof(cubes_scene_t),
               (u8 *)scenePool->permBase + sizeof(cubes_scene_t));
    
    arena_init(&scene->arenaTemp, scenePool->tempSize, (u8 *)scenePool->tempBase);
    
    
    memory_arena_t *arena = &scene->arenaPerm;
    
    cubes_scene_load_texture_atlas(scene);
    
#if 0
    scene->noiseOctave = 3;
    scene->noiseBias = 0.8f;
#else
    
    
    // Open simplex noise test
    open_simplex_noise(0, &scene->noiseContext);
    
    
#endif
    
    // "Player"
    scene->cameraTarget = transform_get_default();
    scene->cameraTarget.translation.y = 2;
    scene->cameraTarget.rotation.x = 0;
    scene->cameraTarget.rotation.y = 3.1415f;
    
    // 3rd person camera following player
    scene->camera.targetTransform = &scene->cameraTarget;
    scene->camera.distanceFromTarget = 20,
    
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
    
    scene->renderPasses = push_array(arena, 3, gl_renderpass_t, 4);
    // Static cubes
    scene->renderPasses[0] = opengl_make_renderpass(scene->cubesVertexBuffer, gl_primitive_triangles, 
                                                    0, scene->atlasTexture, 0, cameraView, perspectiveProj, &textureShader);
    // Lines
    scene->renderPasses[1] = opengl_make_renderpass(scene->linesVertexBuffer, gl_primitive_lines,
                                                    0, 0, 2, cameraView, perspectiveProj, &lineShader);
    // Debug text
    scene->renderPasses[2] = opengl_make_renderpass(scene->textVertexBuffer, gl_primitive_triangles,
                                                    0, globalFontTexture, 0, noView, orthographicProj, &textureShader);
}

internal void
cubes_scene_update(memory_pool_t *scenePool, f32 elapsedTime)
{
    cubes_scene_t *scene = (cubes_scene_t *)scenePool->permBase;
    
    memory_arena_t *arena = &scene->arenaPerm;
    
    arena_init(&scene->arenaTemp, scenePool->tempSize, (u8 *)scenePool->tempBase);
    
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
                if (scene->noiseHeight < 32)
                {
                    scene->noiseHeight += 1;
                    calculateNoise = 1;
                }
            }
            else if (os.keyboard.buttons[KEY_MINUS].pressed)
            {
                if (scene->noiseHeight > 1)
                {
                    scene->noiseHeight -= 1;
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
                scene->noiseBias += 10.0f;
                calculateNoise = 1;
            }
            else if (os.keyboard.buttons[KEY_MINUS].pressed)
            {
                scene->noiseBias -= 10.0f;
                calculateNoise = 1;
            }
        }
    }
    
    if ((os.keyboard.buttons[KEY_CONTROL].down &&
         os.keyboard.buttons[KEY_ENTER].pressed) || calculateNoise)
    {
        // reset vertex buffer
        scene->cubesVertexBuffer->indexCount = 0;
        scene->cubesVertexBuffer->vertexCount = 0;
        
        // recalculate chunks
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
            scene->camera.angleAroundTarget += elapsedTime * 0.1f * deltaMousePos.x;
        }
    }
    
    // Begin temporary memory
    temp_memory_t mem = begin_temp_memory(arena);
    
    
    // Debug text
    {
        scene->textVertexBuffer->vertexCount = 0;
        scene->textVertexBuffer->indexCount = 0;
        
        u32 debugTextCount = 2;
        
        stbtt_print(arena, scene->textVertexBuffer, 0, (debugTextCount++)*30.0f, "Cubes test scene");
        
        char labelHeight[256];
        label_make_f32(labelHeight, sizeof(labelHeight), "Height: ", (f32)scene->noiseHeight);
        stbtt_print(arena, scene->textVertexBuffer, 0, (debugTextCount++)*30.0f, labelHeight);
        
        char labelBias[256];
        label_make_f32(labelBias, sizeof(labelBias), "Bias: ", scene->noiseBias);
        stbtt_print(arena, scene->textVertexBuffer, 0, (debugTextCount++)*30.0f, labelBias);
        
        opengl_upload_vbuffer_data(scene->textVertexBuffer);
    }
    
    // End temporary memory
    end_temp_memory(mem);
    
    camera_update(&scene->camera);
    
    cubes_scene_update_renderpasses(scene->renderPasses, &scene->camera);
    
    
    opengl_execute_renderpasses(&os.gl, scene->renderPasses, 3);
}