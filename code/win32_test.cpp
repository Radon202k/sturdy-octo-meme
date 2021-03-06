#include "renderer.h"
#include "voxel.h"

#include "renderer.cpp"
#include "voxel.cpp"

enum material
{
    material_null,
    material_air,
    material_dirt,
    material_stone,
    material_cobblestone,
};

struct globals
{
    hnMandala permanent;
    hnMandala temporary;
    
    hnAudio *audio;
    
    hnRandomSeries entropy;
    
    mcRenderer *renderer;
    f64 time;
    
    f32 dtHistory[200];
    u32 dtHistoryIndex;
    
    voxel_map overworld;
    camera cam;
    
    b32 capturingMouse;
    
    v2 lastMouseP;
};

global globals app;

#include "memory.cpp"
#include "debug.cpp"

internal void
hnResizeCallback(void)
{
    app.renderer->cubes.scissor = hnRectMinMax({0,0},win32.clientDim);
    app.renderer->ortho2d.scissor = hnRectMinMax({0,0},win32.clientDim);
}

internal void
hnActivateCallback(b32 active)
{
    if (active)
    {
        app.capturingMouse = true;
        captureMouse();
    }
    else
    {
        app.capturingMouse = false;
        stopMouseCpature();
    }
}

internal void
initWorld(void)
{
    app.cam.yaw = -90.0f;
    app.cam.pitch = 0;
    
    app.cam.pos = {0,700,0};
    app.cam.front = {0,0,-1};
    app.cam.up = {0,1,0};
    
    app.entropy = hnRandomSeed(123);
    
    app.lastMouseP = win32.input.mouse.pos;
    
    app.overworld.permanent = &app.permanent;
    memset(app.overworld.hash, 0, sizeof(chunk *) * arrayCount(app.overworld.hash));
    
    app.overworld.viewDist = {10,20,10};
    app.overworld.chunkShift = 4;
    app.overworld.chunkDim = (1 << app.overworld.chunkShift);
    app.overworld.chunkMask = app.overworld.chunkDim - 1;
    
    captureMouse();
    app.capturingMouse = true;
}

internal void
updateCameraInput(void)
{
    hnKeyboard *key = &win32.input.keyboard;
    f32 speed = 30.0f;
    f32 dt = app.renderer->backend->dt;
    
    if (key->w.down)
    {
        app.cam.pos += dt * speed * app.cam.front;
    }
    
    if (key->s.down)
    {
        app.cam.pos -= dt * speed * app.cam.front;
    }
    
    if (key->a.down)
    {
        app.cam.pos -= dt * speed * normalize(cross(app.cam.front, app.cam.up));
    }
    
    if (key->d.down)
    {
        app.cam.pos += dt * speed * normalize(cross(app.cam.front, app.cam.up));
    }
    
    if (key->space.down)
    {
        app.cam.pos += dt * speed * app.cam.up;
    }
    
    if (key->shift.down)
    {
        app.cam.pos -= dt * speed * app.cam.up;
    }
    
    //
    
    hnMouse *mouse = &win32.input.mouse;
    v2 center = 0.5f * win32.clientDim;
    v2 deltaP = mouse->pos - center;
    
    if (app.capturingMouse)
    {
        SetCursorPos((s32)center.x, (s32)center.y);
    }
    
    f32 sensitivity = 10.0f;
    app.cam.yaw += sensitivity * dt * deltaP.x;
    app.cam.pitch -= sensitivity * dt * deltaP.y;
    
    if (app.cam.pitch > 89.0f)
    {
        app.cam.pitch = 89.0f;
    }
    else if (app.cam.pitch < -89.0f)
    {
        app.cam.pitch = -89.0f;
    }
}

internal void
updateWorld(void)
{
    hnMouse *mouse = &win32.input.mouse;
    hnKeyboard *key = &win32.input.keyboard;
    f32 dt = app.renderer->backend->dt;
    
    if (key->escape.down)
    {
        app.renderer->backend->shouldClose = true;
    }
    
    updateCameraInput();
    
    updateChunkLoading(app.renderer, &app.overworld, app.cam.pos);
}

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
    allocMemoryPools();
    
    app.renderer = initRenderer(&app.permanent, &app.temporary);
    
    initWorld();
    
    while (!app.renderer->backend->shouldClose)
    {
        hnPrepareFrame(app.renderer->backend, 0);
        
        app.renderer->ortho2dVb->index = 0;
        app.renderer->ortho2dIb->index = 0;
        
        app.renderer->linesVb->index = 0;
        app.renderer->linesIb->index = 0;
        
        updateWorld();
        
        updateDebugDisplay();
        
        hnUploadGpuBuffer(app.renderer->backend, app.renderer->ortho2dVb);
        hnUploadGpuBuffer(app.renderer->backend, app.renderer->ortho2dIb);
        
        hnUploadGpuBuffer(app.renderer->backend, app.renderer->linesVb);
        hnUploadGpuBuffer(app.renderer->backend, app.renderer->linesIb);
        
        updateProjectionMatrices(app.renderer, &app.cam);
        
        // Lines
        hnExecuteRenderPass(app.renderer->backend, &app.renderer->lines3d, 
                            app.renderer->linesVb, app.renderer->linesIb, true, hnDEEPPINK);
        
        
        v3 chunkLoadingCenter = app.overworld.currentCenter;
        
        v3 extentsMin = chunkLoadingCenter - 0.5f*app.overworld.viewDist; // oldMin
        v3 extentsMax = chunkLoadingCenter + 0.5f*app.overworld.viewDist; // oldMax
        
        u32 renderPassIndex = 0;
        for (s32 chunkX = (s32)extentsMin.x;
             chunkX < extentsMax.x;
             ++chunkX)
        {
            for (s32 chunkY = (s32)extentsMin.y;
                 chunkY < extentsMax.y;
                 ++chunkY)
            {
                for (s32 chunkZ = (s32)extentsMin.z;
                     chunkZ < extentsMax.z;
                     ++chunkZ)
                {
                    chunk *c = getChunk(&app.overworld, chunkX, chunkY, chunkZ);
                    if (c && c->active)
                    {
                        hnExecuteRenderPass(app.renderer->backend, &app.renderer->cubes, 
                                            c->vb, c->ib, false, {});
                    }
                }
            }
        }
        
        hnExecuteRenderPass(app.renderer->backend, &app.renderer->ortho2d, 
                            app.renderer->ortho2dVb, app.renderer->ortho2dIb, false, {});
        
        hnPresentFrame(app.renderer->backend, 0);
    }
    
    ReleaseCapture();
}