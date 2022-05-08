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
    
    memcpy((u8 *)r->cubes.pass.uniforms[0].data, proj.forward.e, sizeof(proj.forward));
    memcpy((u8 *)r->cubes.pass.uniforms[1].data, view.e, sizeof(view));
    
    // 2d orthographic projection (pixel top down)
    mat4x4_inv ortho = hnOrthographicProjection(win32.clientDim);
    memcpy((u8 *)r->ortho2d.pass.uniforms[0].data, ortho.forward.e, sizeof(ortho.forward));
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

internal mcRenderer *
initRenderer(hnMandala *permanent, hnMandala *temporary)
{
    mcRenderer *r = hnPushStruct(permanent, mcRenderer);
    
    hnTempMemory tempMem = hnBeginTempMemory(temporary);
    
    r->backend = hnInitRenderer(permanent, temporary, 800, 600, "Hello, World!");
    // glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    
    r->cubes.vb = hnMakeVertexBuffer(r->backend, megabytes(512), sizeof(f32)*13);
    r->cubes.ib = hnMakeIndexBuffer(r->backend, megabytes(128));
    
    hnSetInputLayout(&r->cubes.vb, 0, GL_FLOAT, 3, offsetof(vertex3d, pos));
    hnSetInputLayout(&r->cubes.vb, 1, GL_FLOAT, 3, offsetof(vertex3d, uv));
    hnSetInputLayout(&r->cubes.vb, 2, GL_FLOAT, 3, offsetof(vertex3d, nor));
    hnSetInputLayout(&r->cubes.vb, 3, GL_FLOAT, 4, offsetof(vertex3d, col));
    
    r->ortho2d.vb = hnMakeVertexBuffer(r->backend, megabytes(4), sizeof(f32)*10);
    r->ortho2d.ib = hnMakeIndexBuffer(r->backend, megabytes(1));
    
    hnSetInputLayout(&r->ortho2d.vb, 0, GL_FLOAT, 3, offsetof(vertex2d, pos));
    hnSetInputLayout(&r->ortho2d.vb, 1, GL_FLOAT, 3, offsetof(vertex2d, uv));
    hnSetInputLayout(&r->ortho2d.vb, 2, GL_FLOAT, 4, offsetof(vertex2d, col));
    
    {
        char vertexShader[] = R"VSHADER(
            #version 450 core
    
            layout (location=0) in vec3 inPos;
            layout (location=1) in vec3 inUV;
            layout (location=2) in vec3 inNor;
            layout (location=3) in vec4 inCol;
            
            layout (location=0) uniform mat4 proj;
            layout (location=1) uniform mat4 view;
            
            out gl_PerVertex { vec4 gl_Position; };
            out vec3 fragP;
out vec3 uv;
            out vec3 normal;
            out vec4 color;
            
            void main()
            {
                gl_Position = vec4(inPos, 1) * view * proj;
                fragP = inPos;
uv = inUV;
                 normal = inNor;
                color = inCol;
            }
            )VSHADER";
        
        char fragShader[] = R"FSHADER(
            #version 450 core
    
            in vec3 fragP;
in vec3 uv;
            in vec3 normal;
            in vec4 color;
            
            layout (location=0)
            out vec4 outColor;
            
layout (binding=0)
            uniform sampler2DArray textureArray;
            
            void main()
            {
vec3 lightP = vec3(10,30,20);
vec3 lightColor = vec3(1,1,0);

float ambientStrength = 0.1;
vec3 ambient = ambientStrength * lightColor;

vec3 lightDir = normalize(lightP - fragP);
float diff = max(dot(normal, lightDir), 0.0);
vec3 diffuse = diff * lightColor;

                 vec3 lightResult = (ambient + diffuse);
                
vec4 texelColor = texture(textureArray, uv);

outColor = vec4(lightResult, 1) * color * texelColor;
            }
            )FSHADER";
        
        r->shader3d = hnMakeShader(r->backend, vertexShader, fragShader);
    }
    
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
    }
    
    r->cubes.pass = {};
    r->cubes.pass.primitive = hnTRIANGLES;
    r->cubes.pass.shader = &r->shader3d;
    r->cubes.pass.target = r->backend->defaultTarget;
    r->cubes.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    r->cubes.pass.blendEnabled = true;
    r->cubes.pass.blendType = hnPREMULTIPLIED_ALPHA;
    r->cubes.pass.depthTestEnabled = true;
    r->cubes.pass.depthTestType = hnLESS_EQUAL;
    
    r->cubes.pass.uniformCount = 2;
    r->cubes.pass.uniforms = hnPushArray(permanent, r->cubes.pass.uniformCount, hnGpuBuffer);
    
    r->cubes.pass.uniforms[0] = hnMakeUniformBuffer(r->backend, r->shader3d.vShader, 0, sizeof(f32)*4*4);
    r->cubes.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    r->cubes.pass.uniforms[1] = hnMakeUniformBuffer(r->backend, r->shader3d.vShader, 1, sizeof(f32)*4*4);
    r->cubes.pass.uniforms[1].type = hnUNIFORM_MAT4FV;
    
    //
    
    r->ortho2d.pass = {};
    r->ortho2d.pass.primitive = hnTRIANGLES;
    r->ortho2d.pass.shader = &r->shader2d;
    r->ortho2d.pass.target = r->backend->defaultTarget;
    r->ortho2d.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    r->ortho2d.pass.blendEnabled = true;
    r->ortho2d.pass.blendType = hnPREMULTIPLIED_ALPHA;
    r->ortho2d.pass.depthTestEnabled = false;
    r->ortho2d.pass.depthTestType = hnLESS_EQUAL;
    
    r->ortho2d.pass.uniformCount = 1;
    r->ortho2d.pass.uniforms = hnPushArray(permanent, r->ortho2d.pass.uniformCount, hnGpuBuffer);
    
    r->ortho2d.pass.uniforms[0] = hnMakeUniformBuffer(r->backend, r->shader2d.vShader, 0, sizeof(f32)*4*4);
    r->ortho2d.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    
    //
    
    r->textureArray = hnMakeTexture2DArray(r->backend, 1024, 1024, 32, hnNEAREST);
    r->font = hnInitFont(r->backend, &r->textureArray, &r->fontSprite, "C:\\Windows\\Fonts\\arial.ttf",16);
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->white, "images/white.png");
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->circle, "images/circle.png");
    hnLoadTexture2DIntoArray(r->backend, &r->textureArray, &r->atlas, "images/atlas.png");
    glGenerateTextureMipmap(r->textureArray.handle);
    
    //
    
    r->cubes.pass.textureUnit = 0;
    r->cubes.pass.textureHandle = r->textureArray.handle;
    
    r->ortho2d.pass.textureUnit = 0;
    r->ortho2d.pass.textureHandle = r->textureArray.handle;
    
    //
    
    r->dirt = hnMakeSprite(r->atlas,16,{2,0},{3,1});
    r->stone = hnMakeSprite(r->atlas,16,{1,0},{2,1});
    r->snow = hnMakeSprite(r->atlas,16,{2,4},{3,5});
    
    hnEndTempMemory(tempMem);
    
    captureMouse();
    
    return r;
}