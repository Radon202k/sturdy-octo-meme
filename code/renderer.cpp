internal mat4x4
cameraViewMatrix(camera *cam)
{
    v3 dir =
    {
        cosf(toRadians(cam->yaw)) * cosf(toRadians(cam->pitch)),
        sinf(toRadians(cam->pitch)),
        sinf(toRadians(cam->yaw)) * cosf(toRadians(cam->pitch))
    };
    
    cam->front = normalize(dir);
    
    mat4x4 result = hnLookAtMatrix(cam->pos, cam->pos + cam->front, cam->up);
    
    return result;
}

internal void
updateProjectionMatrices(mcRenderer *r, camera *cam)
{
    // 3d perspectivep rojection and view matrix
    f32 ar = win32.clientDim.x / win32.clientDim.y;
    f32 focalLength = 45;
    
    mat4x4_inv proj = hnPerspectiveProjection(ar, focalLength, 0.1f, 1000.0f);
    mat4x4 view = cameraViewMatrix(cam);
    
    memcpy((u8 *)r->cubes.uniforms[0].data, proj.forward.e, sizeof(proj.forward));
    memcpy((u8 *)r->cubes.uniforms[1].data, view.e, sizeof(view));
    
    memcpy((u8 *)r->lines3d.uniforms[0].data, proj.forward.e, sizeof(proj.forward));
    memcpy((u8 *)r->lines3d.uniforms[1].data, view.e, sizeof(view));
    
    
    // 2d orthographic projection (pixel top down)
    mat4x4_inv ortho = hnOrthographicProjection(win32.clientDim);
    memcpy((u8 *)r->ortho2d.uniforms[0].data, ortho.forward.e, sizeof(ortho.forward));
}

internal void
captureMouse(void)
{
    SetCapture(win32.window);
    ShowCursor(0);
    
    RECT clipRect;
    GetClientRect(win32.window, &clipRect);
    ClientToScreen(win32.window, (POINT *)&clipRect.left);
    ClientToScreen(win32.window, (POINT *)&clipRect.right);
    ClipCursor(&clipRect);
}

internal void
stopMouseCpature(void)
{
    ReleaseCapture();
    ShowCursor(1);
}

internal void
make3dCubesShaderAndRenderPass(mcRenderer *r, u32 textureUnit, u32 textureHandle)
{
    char vertexShader[] = R"VSHADER(
        #version 450 core

        layout (location=0) in vec3 inPos;
        layout (location=1) in vec3 inUV;
        layout (location=2) in vec3 inNor;
        
        layout (location=0) uniform mat4 proj;
        layout (location=1) uniform mat4 view;
        
        out gl_PerVertex { vec4 gl_Position; };
        out vec3 fragP;
out vec3 uv;
        out vec3 normal;
        
        void main()
        {
            gl_Position = vec4(inPos, 1) * view * proj;
            fragP = inPos;
uv = inUV;
             normal = inNor;
            }
        )VSHADER";
    
    char fragShader[] = R"FSHADER(
        #version 450 core

        in vec3 fragP;
in vec3 uv;
        in vec3 normal;
        
        layout (location=0)
        out vec4 outColor;
        
layout (binding=0)
        uniform sampler2DArray textureArray;
        
        void main()
        {
vec3 lightP = vec3(1000,1000,3000);
vec3 lightColor = vec3(1,1,1);

float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;

vec3 lightDir = normalize(lightP - fragP);
float diff = max(dot(normal, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

             vec3 lightResult = (ambient + diffuse);
            
vec4 texelColor = texture(textureArray, uv);

outColor = vec4(lightResult, 1) * texelColor;
        }
        )FSHADER";
    
    r->shader3d = hnMakeShader(r->backend, vertexShader, fragShader);
    
    r->cubes = {};
    r->cubes.primitive = hnTRIANGLES;
    r->cubes.shader = &r->shader3d;
    r->cubes.target = r->backend->defaultTarget;
    r->cubes.scissor = hnRectMinMax({0,0}, win32.clientDim);
    r->cubes.blendEnabled = true;
    r->cubes.blendType = hnPREMULTIPLIED_ALPHA;
    r->cubes.depthTestEnabled = true;
    r->cubes.depthTestType = hnLESS_EQUAL;
    
    r->cubes.uniformCount = 2;
    r->cubes.uniforms = hnPushArray(r->backend->perm, r->cubes.uniformCount, hnGpuBuffer);
    
    r->cubes.uniforms[0] = hnMakeUniformBuffer(r->backend, hnUNIFORM_MAT4FV, r->shader3d.vShader, 0, sizeof(f32)*4*4);
    r->cubes.uniforms[1] = hnMakeUniformBuffer(r->backend, hnUNIFORM_MAT4FV, r->shader3d.vShader, 1, sizeof(f32)*4*4);
    
    r->cubes.textureUnit = textureUnit;
    r->cubes.textureHandle = textureHandle;
}


internal void
make3dLinesShaderAndRenderPass(mcRenderer *r)
{
    char vertexShader[] = R"VSHADER(
        #version 450 core

        layout (location=0) in vec3 inPos;
        layout (location=1) in vec4 inCol;
        
        layout (location=0) uniform mat4 proj;
        layout (location=1) uniform mat4 view;
        
        out gl_PerVertex { vec4 gl_Position; };
        
out vec4 color;

        void main()
        {
            gl_Position = vec4(inPos, 1) * view * proj;
            color = inCol;
}
        )VSHADER";
    
    char fragShader[] = R"FSHADER(
        #version 450 core

in vec4 color;

        layout (location=0)
        out vec4 outColor;
        
void main()
        {
outColor = color;
        }
        )FSHADER";
    
    r->lineShader3d = hnMakeShader(r->backend, vertexShader, fragShader);
    
    r->lines3d = {};
    r->lines3d.primitive = hnLINES;
    r->lines3d.shader = &r->lineShader3d;
    r->lines3d.target = r->backend->defaultTarget;
    r->lines3d.scissor = hnRectMinMax({0,0}, win32.clientDim);
    r->lines3d.blendEnabled = true;
    r->lines3d.blendType = hnPREMULTIPLIED_ALPHA;
    r->lines3d.depthTestEnabled = true;
    r->lines3d.depthTestType = hnLESS_EQUAL;
    r->lines3d.lineWidth = 10;
    
    r->lines3d.uniformCount = 2;
    r->lines3d.uniforms = hnPushArray(r->backend->perm, r->lines3d.uniformCount, hnGpuBuffer);
    
    r->lines3d.uniforms[0] = hnMakeUniformBuffer(r->backend, hnUNIFORM_MAT4FV, r->lineShader3d.vShader, 0, sizeof(f32)*4*4);
    r->lines3d.uniforms[1] = hnMakeUniformBuffer(r->backend, hnUNIFORM_MAT4FV, r->lineShader3d.vShader, 1, sizeof(f32)*4*4);
}


internal void
make2dShaderAndRenderPass(mcRenderer *r, u32 textureUnit, u32 textureHandle)
{
    char vertexShader[] = R"VSHADER(
        #version 450 core

        layout (location=0) in vec3 inPos;
        layout (location=1) in vec3 inUV;
        layout (location=2) in vec4 inCol;
        
        layout (location=0) uniform mat4 proj;
        
        out gl_PerVertex { vec4 gl_Position; };
        out vec3 uv;
        out vec4 color;
        
        void main()
        {
            gl_Position = vec4(inPos, 1) * proj;
            uv = inUV;
             color = inCol;
        }
        )VSHADER";
    
    char fragShader[] = R"FSHADER(
        #version 450 core

        in vec3 uv;
        in vec4 color;
        
        layout (location=0)
        out vec4 outColor;
        
layout (binding=0)
        uniform sampler2DArray textureArray;
        
        void main()
        {
vec4 texelColor = texture(textureArray, uv);
outColor = color * texelColor;
        }
        )FSHADER";
    
    r->shader2d = hnMakeShader(r->backend, vertexShader, fragShader);
    
    r->ortho2d = {};
    r->ortho2d.primitive = hnTRIANGLES;
    r->ortho2d.shader = &r->shader2d;
    r->ortho2d.target = r->backend->defaultTarget;
    r->ortho2d.scissor = hnRectMinMax({0,0}, win32.clientDim);
    r->ortho2d.blendEnabled = true;
    r->ortho2d.blendType = hnPREMULTIPLIED_ALPHA;
    r->ortho2d.depthTestEnabled = false;
    r->ortho2d.depthTestType = hnLESS_EQUAL;
    
    r->ortho2d.uniformCount = 1;
    r->ortho2d.uniforms = hnPushArray(r->backend->perm, r->ortho2d.uniformCount, hnGpuBuffer);
    
    r->ortho2d.uniforms[0] = hnMakeUniformBuffer(r->backend, hnUNIFORM_MAT4FV, r->shader2d.vShader, 0, sizeof(f32)*4*4);
    
    r->ortho2d.textureUnit = textureUnit;
    r->ortho2d.textureHandle = textureHandle;
}

internal mcRenderer *
initRenderer(hnMandala *permanent, hnMandala *temporary)
{
    mcRenderer *r = hnPushStruct(permanent, mcRenderer);
    
    hnTempMemory tempMem = hnBeginTempMemory(temporary);
    
    r->backend = hnInitRenderer(permanent, temporary, 800, 600, "Hello, World!");
    // glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    
    r->ortho2dVb = hnMakeVertexBuffer(r->backend, megabytes(4), sizeof(f32)*6 + sizeof(u32));
    r->ortho2dIb = hnMakeIndexBuffer(r->backend, megabytes(1));
    hnSetInputLayout(r->ortho2dVb, 0, GL_FLOAT, 3, offsetof(vertex2d, pos), GL_FALSE);
    hnSetInputLayout(r->ortho2dVb, 1, GL_FLOAT, 3, offsetof(vertex2d, uv), GL_FALSE);
    hnSetInputLayout(r->ortho2dVb, 2, GL_UNSIGNED_BYTE, 4, offsetof(vertex2d, col), GL_TRUE);
    
    r->linesVb = hnMakeVertexBuffer(r->backend, megabytes(4), sizeof(f32)*3 + sizeof(u32));
    r->linesIb = hnMakeIndexBuffer(r->backend, megabytes(1));
    hnSetInputLayout(r->linesVb, 0, GL_FLOAT, 3, offsetof(vertex3d_lines, pos), GL_FALSE);
    hnSetInputLayout(r->linesVb, 1, GL_UNSIGNED_BYTE, 4, offsetof(vertex3d_lines, col), GL_TRUE);
    
    // Make texture array
    r->textureArray = hnMakeTexture2DArray(r->backend, 1024, 1024, 32, hnNEAREST);
    
    // Init font (this makes a texture inside the texture array)
    r->font = hnInitFont(r->backend, &r->textureArray, &r->fontSprite, "C:\\Windows\\Fonts\\arial.ttf",16);
    
    // Load textures inside texture array
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->white, "images/white.png");
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->circle, "images/circle.png");
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->atlas, "images/atlas.png");
    
    // Generate mipmaps
    glGenerateTextureMipmap(r->textureArray.handle);
    
    // Make shaders and render passes
    make3dCubesShaderAndRenderPass(r, 0, r->textureArray.handle);
    make3dLinesShaderAndRenderPass(r);
    make2dShaderAndRenderPass(r, 0, r->textureArray.handle);
    
    // Make block sprite uvs
    r->dirt = hnMakeSprite(r->atlas,16,{2,0},{3,1});
    r->stone = hnMakeSprite(r->atlas,16,{1,0},{2,1});
    r->snow = hnMakeSprite(r->atlas,16,{2,4},{3,5});
    
    hnEndTempMemory(tempMem);
    
    return r;
}