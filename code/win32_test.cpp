#include "renderer.h"
#include "voxel.h"
#include "noise.h"

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
    
    
    v2 lastMouseP;
};

global globals app;

#include "memory.cpp"
#include "debug.cpp"

internal void
hnResizeCallback(void)
{
    app.renderer->cubes.pass.scissor = hnRectMinMax({0,0},win32.clientDim);
}

internal void
initWorld(void)
{
    app.cam.yaw = -90.0f;
    app.cam.pitch = 0;
    
    app.cam.pos = {0,30,3};
    app.cam.front = {0,0,-1};
    app.cam.up = {0,1,0};
    
    app.entropy = hnRandomSeed(123);
    
    app.lastMouseP = win32.input.mouse.pos;
    
    app.overworld.permanent = &app.permanent;
    memset(app.overworld.hash, 0, sizeof(chunk *) * arrayCount(app.overworld.hash));
    
    app.overworld.viewDist = 4;
    app.overworld.currentCenter = app.cam.pos;
    app.overworld.maxDistFromCurrentCenter = CHUNK_SIZE;
    
    generateNoise2DAroundP(&app.overworld, app.cam.pos, app.overworld.viewDist, 64);
    
    generateChunksAroundP(app.renderer, &app.overworld, app.cam.pos, app.overworld.viewDist);
}

internal void
updateCameraInput(void)
{
    hnKeyboard *key = &win32.input.keyboard;
    f32 speed = 100.0f;
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
    
    SetCursorPos((s32)center.x, (s32)center.y);
    
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
        
        app.renderer->ortho2d.ib.index = 0;
        app.renderer->ortho2d.vb.index = 0;
        
        updateWorld();
        
        updateDebugDisplay();
        
        hnUploadGpuBuffer(app.renderer->backend, &app.renderer->ortho2d.vb);
        hnUploadGpuBuffer(app.renderer->backend, &app.renderer->ortho2d.ib);
        
        updateProjectionMatrices(app.renderer, &app.cam);
        hnExecuteRenderPass(app.renderer->backend, &app.renderer->cubes.pass, &app.renderer->cubes.vb, &app.renderer->cubes.ib, true, hnDIMGRAY);
        hnExecuteRenderPass(app.renderer->backend, &app.renderer->ortho2d.pass, &app.renderer->ortho2d.vb, &app.renderer->ortho2d.ib, false, {});
        
        hnPresentFrame(app.renderer->backend, 0);
    }
    
    ReleaseCapture();
}