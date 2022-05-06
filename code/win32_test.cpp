#define HANE3D_DEBUG
#define HANE3D_WIN32
#define HANE3D_OPENGL
#include "h:\hane3d\hane3d.h"

struct transform
{
    v3 translation;
    v3 rotation;
    v3 scale;
};

struct camera
{
    v3 pos;
    f32 pitch;
    f32 yaw;
    f32 roll;
};

struct globals
{
    hnMandala permanent;
    hnMandala temporary;
    
    hnRenderer *renderer;
    hnAudio *audio;
    
    hnTexture2DArray textureArray;
    
    hnShader shader;
    
    hnGpuBuffer vb;
    hnGpuBuffer ib;
    hnRenderPass pass;
    
    f64 time;
    
    camera cam;
    transform camTarget;
    
    b32 dragging;
    v2 lastMouseP;
};

global globals app;

internal mat4x4
cameraViewMatrix(camera *cam)
{
    mat4x4 result = hnRotationXMatrix(cam->pitch);
    result = result * hnRotationYMatrix(cam->yaw);
    result = result * hnTranslationMatrix(-cam->pos);
    return result;
}

inline transform
defaultTransform(void)
{
    transform result = 
    {
        {0,0,0},
        {0,0,0},
        {1,1,1},
    };
    return result;
}
internal void
updateProjectionMatrix(void)
{
    f32 ar = win32.clientDim.x / win32.clientDim.y;
    f32 focalLength = 45;
    
    mat4x4_inv proj = hnPerspectiveProjection(ar, focalLength, 0.1f, 100.0f);
    
    mat4x4 view = cameraViewMatrix(&app.cam);
    
    mat4x4 projview = proj.forward * view;
    
    memcpy((u8 *)app.pass.uniforms[0].data, projview.data, sizeof(projview));
}

internal void
hnResizeCallback(void)
{
    app.pass.scissor = hnRectMinMax({0,0},win32.clientDim);
    updateProjectionMatrix();
}

struct vertex3d
{
    v3 pos;
    v2 uv;
    v3 nor;
    v4 col;
};

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
#ifdef HANE3D_DEBUG
    void *platMemAddress = (void *)terabytes(2);
#else
    void *platMemAddress = 0;
#endif
    u32 platPermSize = megabytes(128);
    u32 platTempSize = megabytes(128);
    u32 totalMemSize = platPermSize + platTempSize;
    
    app.permanent = hnInitMandala(platPermSize, hnAllocateMemory(platMemAddress, totalMemSize));
    app.temporary = hnInitMandala(platTempSize, (u8 *)app.permanent.base + platPermSize);
    
    hnTempMemory tempMem = hnBeginTempMemory(&app.temporary);
    app.renderer = hnInitRenderer(&app.permanent, &app.temporary, 800, 600, "Hello, World!");
    // glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    
    app.vb = hnMakeVertexBuffer(app.renderer, megabytes(32), sizeof(f32)*12);
    app.ib = hnMakeIndexBuffer(app.renderer, megabytes(32));
    
    hnSetInputLayout(&app.vb, 0, GL_FLOAT, 3, offsetof(vertex3d, pos));
    hnSetInputLayout(&app.vb, 1, GL_FLOAT, 2, offsetof(vertex3d, uv));
    hnSetInputLayout(&app.vb, 2, GL_FLOAT, 3, offsetof(vertex3d, nor));
    hnSetInputLayout(&app.vb, 3, GL_FLOAT, 4, offsetof(vertex3d, col));
    
    {
        char vertexShader[] = R"VSHADER(
            #version 450 core
    
            layout (location=0) in vec3 inPos;
            layout (location=1) in vec2 inUV;
            layout (location=2) in vec3 inNor;
            layout (location=3) in vec4 inCol;
            
            layout (location=0) uniform mat4 projview;
            
            out gl_PerVertex { vec4 gl_Position; };
            out vec2 uv;
            out vec3 normal;
            out vec4 color;
            
            void main()
            {
                gl_Position = projview * vec4(inPos, 1);
                uv = inUV;
                 normal = inNor;
                color = inCol;
            }
            )VSHADER";
        
        char fragShader[] = R"FSHADER(
            #version 450 core
    
            in vec2 uv;
            in vec3 normal;
            in vec4 color;
            
            layout (location=0)
            out vec4 outColor;
            
            void main()
            {
                vec4 weirdGradient = vec4(uv.x,uv.y,1,1);
                outColor = color;
            }
            )FSHADER";
        
        app.shader = hnMakeShader(app.renderer, vertexShader, fragShader);
    }
    
    app.pass = {};
    app.pass.primitive = hnTRIANGLES;
    app.pass.shader = &app.shader;
    app.pass.target = app.renderer->defaultTarget;
    app.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    app.pass.blendEnabled = true;
    app.pass.blendType = hnBLEND_DEFAULT;
    app.pass.depthTestEnabled = false;
    app.pass.depthTestType = hnLESS_EQUAL;
    
    app.pass.uniformCount = 1;
    app.pass.uniforms = hnPushArray(&app.permanent, app.pass.uniformCount, hnGpuBuffer);
    app.pass.uniforms[0] = hnMakeUniformBuffer(app.renderer, app.shader.vShader, 0, sizeof(f32)*4*4);
    app.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    
    updateProjectionMatrix();
    
    hnEndTempMemory(tempMem);
    
    app.cam.pos = {0,0,10};
    
    app.cam.pitch = 0;
    app.cam.yaw = 0;
    app.cam.roll = 0;
    
    while (!app.renderer->shouldClose)
    {
        hnPrepareFrame(app.renderer, 0);
        app.vb.index = 0;
        app.ib.index = 0;
        
        hnMouse *mouse = &win32.input.mouse;
        hnKeyboard *key = &win32.input.keyboard;
        
        f32 dt = app.renderer->dt;
        
        if (mouse->left.down && !app.dragging)
        {
            app.dragging = true;
            app.lastMouseP = mouse->pos;
        }
        
        if (app.dragging && !mouse->left.down)
        {
            app.dragging = false;
        }
        
        if (app.dragging)
        {
            v2 deltaP = mouse->pos - app.lastMouseP;
            app.lastMouseP = mouse->pos;
            
            if (key->control.down)
            {
                app.cam.pos.z += dt * deltaP.y;
            }
            else
            {
                app.cam.yaw += dt * deltaP.x;
                app.cam.pitch += dt * -deltaP.y;
            }
        }
        
        v2 dim = v2{64,64} + sinf((f32)app.time)*v2{128,128};
        
        v3 scale = 0.2f*v3{1,1,1};
        f32 margin = 0.1f;
        
        s32 chunkRadius = 2;
        for (s32 y = -chunkRadius;
             y < chunkRadius;
             ++y)
        {
            for (s32 x = -chunkRadius;
                 x < chunkRadius;
                 ++x)
            {
                hnPushCubeIndexed(&app.vb, &app.ib, {}, {x*(scale.x+margin),y*(scale.y+margin),0}, scale, hnGOLD);
                
            }
        }
        
        hnUploadGpuBuffer(app.renderer, &app.vb);
        hnUploadGpuBuffer(app.renderer, &app.ib);
        
        
        updateProjectionMatrix();
        hnExecuteRenderPass(app.renderer, &app.pass, &app.vb, &app.ib, true, hnDIMGRAY);
        
        hnPresentFrame(app.renderer, 0);
        
        app.time += app.renderer->dt;
    }
}