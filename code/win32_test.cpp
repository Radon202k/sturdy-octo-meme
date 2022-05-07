#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "h:\hane3d\libs\stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "h:\hane3d\libs\stb_truetype.h"

#define HANE3D_DEBUG
#define HANE3D_WIN32
#define HANE3D_OPENGL
#define HANE3D_STB_IMAGE
#define HANE3D_STB_TRUETYPE
#include "h:\hane3d\hane3d.h"

#define CHUNK_SIZE v3{8,8,32}

enum material
{
    material_null,
    material_air,
    material_dirt,
    material_stone,
    material_cobblestone,
};

struct chunk
{
    s32 x;
    s32 y;
    s32 z;
    
    u32 *voxels;
};

struct voxel_map
{
    hnMandala *permanent;
    chunk *hash[2048];
};

inline u32
getHashIndex(voxel_map *map, s32 x, s32 y, s32 z)
{
    assert(arrayCount(map->hash) % 2 == 0);
    
    assert(x < 100000);
    assert(y < 100000);
    assert(z < 100000);
    // TODO: Better hash function !!
    u32 hashIndex = (u32)(x * 17 + y * 39 + z * 121);
    return hashIndex % arrayCount(map->hash);
}

inline chunk *
getChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getHashIndex(map, x, y, z);
    chunk *result = map->hash[hashIndex];
    
    if (result)
    {
        assert(result->x < 100000);
        assert(result->y < 100000);
        assert(result->z < 100000);
    }
    
    return result;
}

inline s32
getVoxelIndex(s32 x, s32 y, s32 z)
{
    s32 result = (s32)(z*CHUNK_SIZE.x*CHUNK_SIZE.y + y*CHUNK_SIZE.x + x);
    return result;
}

inline s32
getVoxel(chunk *c, s32 x, s32 y, s32 z)
{
    s32 index = getVoxelIndex(x, y, z);
    s32 result = c->voxels[index];
    return result;
}

inline void
setVoxel(chunk *c, s32 x, s32 y, s32 z, u32 value)
{
    s32 index = getVoxelIndex(x, y, z);
    c->voxels[index] = value;
}

inline chunk *
generateChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getHashIndex(map, x, y, z);
    
    map->hash[hashIndex] = hnPushStruct(map->permanent, chunk);
    memset(map->hash[hashIndex], 0, sizeof(chunk));
    
    chunk *result = map->hash[hashIndex];
    
    result->x = x;
    result->y = y;
    result->z = z;
    
    s32 voxelCount = (s32)(CHUNK_SIZE.x*CHUNK_SIZE.y*CHUNK_SIZE.z);
    result->voxels = hnPushArray(map->permanent, 
                                 voxelCount,
                                 u32);
    
    memset(result->voxels, 0, voxelCount*4);
    f32 maxHeight = 32;
    
    for (s32 voxelZ = 0;
         voxelZ < CHUNK_SIZE.z;
         ++voxelZ)
    {
        for (s32 voxelY = 0;
             voxelY < CHUNK_SIZE.y;
             ++voxelY)
        {
            for (s32 voxelX = 0;
                 voxelX < CHUNK_SIZE.x;
                 ++voxelX)
            {
                u32 cubeMaterial = 1;
                
                f32 compX = (f32)(x*CHUNK_SIZE.x + voxelX);
                f32 compY = (f32)(y*CHUNK_SIZE.y + voxelY);
                f32 compZ = (f32)(z*CHUNK_SIZE.z + voxelZ);
                
                f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compY,0.08f,6));
                
                if (compZ <= height)
                {
                    if (compZ > 20)
                    {
                        cubeMaterial = 3;
                    }
                    else if (compZ > 10)
                    {
                        cubeMaterial = 2;
                    }
                    
                    setVoxel(result, voxelX, voxelY, voxelZ, cubeMaterial);
                }
                else
                {
                    setVoxel(result, voxelX, voxelY, voxelZ, 0);
                }
            }
        }
    }
    
    return result;
}

struct transform
{
    v3 translation;
    v3 rotation;
    v3 scale;
};

struct camera
{
    v3 pos;
    v3 front;
    v3 up;
    
    f32 yaw;
    f32 pitch;
};

struct globals
{
    hnMandala permanent;
    hnMandala temporary;
    
    hnRenderer *renderer;
    hnAudio *audio;
    
    hnShader shader;
    hnFont font;
    
    hnTexture2DArray textureArray;
    hnSprite fontSprite;
    hnSprite white;
    hnSprite circle;
    
    hnSprite atlas;
    // Blocks inside atlas
    hnSprite dirt;
    hnSprite stone;
    hnSprite snow;
    hnSprite cobblestone;
    
    hnGpuBuffer vb;
    hnGpuBuffer ib;
    hnRenderPass pass;
    
    hnRandomSeries entropy;
    
    f64 time;
    
    voxel_map overworld;
    camera cam;
    
    v2 lastMouseP;
};

global globals app;

internal mat4x4
cameraViewMatrix(camera *cam)
{
    // Note: cam dir points in the opposite direction of the camera
    // That is so that the z axis is negated and we can specify
    // positive values for the z instead of negative ones.
    
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
    
    mat4x4_inv proj = hnPerspectiveProjection(ar, focalLength, 0.1f, 1000.0f);
    mat4x4 view = cameraViewMatrix(&app.cam);
    
    memcpy((u8 *)app.pass.uniforms[0].data, proj.forward.e, sizeof(proj.forward));
    memcpy((u8 *)app.pass.uniforms[1].data, view.e, sizeof(view));
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
    v3 uv;
    v3 nor;
    v4 col;
};

internal b32
hasSpaceAround(chunk *c, s32 x, s32 y, s32 z)
{
    b32 result = false;
    
    // If it is in the boundary of the chunk
    if ((x == 0 || x == 7) || (y == 0 || y == 7) || (z == 0 || z == 7))
    {
        result = true;
    }
    else
    {
        if (getVoxel(c, x-1, y, z) == 0) // Left
        {
            result = true;
        }
        else if (getVoxel(c, x+1, y, z) == 0) // Right
        {
            result = true;
        }
        else if (getVoxel(c, x, y-1, z) == 0) // Below
        {
            result = true;
        }
        else if (getVoxel(c, x, y+1, z) == 0) // Above
        {
            result = true;
        }
        else if (getVoxel(c, x, y, z-1) == 0) // Font
        {
            result = true;
        }
        else if (getVoxel(c, x, y, z+1) == 0) // Back
        {
            result = true;
        }
    }
    
    return result;
}

int WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR cmdLine, int cmdShow)
{
#ifdef HANE3D_DEBUG
    void *platMemAddress = (void *)terabytes(2);
#else
    void *platMemAddress = 0;
#endif
    u32 platPermSize = gigabytes(2);
    u32 platTempSize = megabytes(512);
    u32 totalMemSize = platPermSize + platTempSize;
    
    app.permanent = hnInitMandala(platPermSize, hnAllocateMemory(platMemAddress, totalMemSize));
    app.temporary = hnInitMandala(platTempSize, (u8 *)app.permanent.base + platPermSize);
    
    hnTempMemory tempMem = hnBeginTempMemory(&app.temporary);
    app.renderer = hnInitRenderer(&app.permanent, &app.temporary, 800, 600, "Hello, World!");
    // glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
    
    app.vb = hnMakeVertexBuffer(app.renderer, megabytes(256), sizeof(f32)*13);
    app.ib = hnMakeIndexBuffer(app.renderer, megabytes(256));
    
    hnSetInputLayout(&app.vb, 0, GL_FLOAT, 3, offsetof(vertex3d, pos));
    hnSetInputLayout(&app.vb, 1, GL_FLOAT, 3, offsetof(vertex3d, uv));
    hnSetInputLayout(&app.vb, 2, GL_FLOAT, 3, offsetof(vertex3d, nor));
    hnSetInputLayout(&app.vb, 3, GL_FLOAT, 4, offsetof(vertex3d, col));
    
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
        
        app.shader = hnMakeShader(app.renderer, vertexShader, fragShader);
    }
    
    app.pass = {};
    app.pass.primitive = hnTRIANGLES;
    app.pass.shader = &app.shader;
    app.pass.target = app.renderer->defaultTarget;
    app.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    app.pass.blendEnabled = true;
    app.pass.blendType = hnBLEND_DEFAULT;
    app.pass.depthTestEnabled = true;
    app.pass.depthTestType = hnLESS_EQUAL;
    
    app.pass.uniformCount = 2;
    app.pass.uniforms = hnPushArray(&app.permanent, app.pass.uniformCount, hnGpuBuffer);
    
    app.pass.uniforms[0] = hnMakeUniformBuffer(app.renderer, app.shader.vShader, 0, sizeof(f32)*4*4);
    app.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    app.pass.uniforms[1] = hnMakeUniformBuffer(app.renderer, app.shader.vShader, 1, sizeof(f32)*4*4);
    app.pass.uniforms[1].type = hnUNIFORM_MAT4FV;
    
    app.textureArray = hnMakeTexture2DArray(app.renderer, 1024, 1024, 32, hnNEAREST);
    
    app.font = hnInitFont(app.renderer, &app.textureArray, &app.fontSprite,
                          "C:\\Windows\\Fonts\\arial.ttf",32);
    
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.white, "images/white.png", hnNEAREST);
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.circle, "images/circle.png", hnNEAREST);
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.atlas, "images/atlas.png", hnNEAREST);
    
    app.dirt = hnMakeSprite(app.atlas,16,{2,0},{3,1});
    app.stone = hnMakeSprite(app.atlas,16,{1,0},{2,1});
    app.snow = hnMakeSprite(app.atlas,16,{2,4},{3,5});
    
    glGenerateTextureMipmap(app.textureArray.handle);
    
    app.pass.textureUnit = 0;
    app.pass.textureHandle = app.textureArray.handle;
    
    hnEndTempMemory(tempMem);
    
    
    app.cam.yaw = -90.0f;
    app.cam.pitch = 0;
    
    app.cam.pos = {0,0,3};
    app.cam.front = {0,0,-1};
    app.cam.up = {0,1,0};
    
    app.entropy = hnRandomSeed(123);
    
    app.lastMouseP = win32.input.mouse.pos;
    
    app.overworld.permanent = &app.permanent;
    memset(app.overworld.hash, 0, sizeof(chunk *) * arrayCount(app.overworld.hash));
    
    // TODO: Z Layers!
    for (s32 chunkX = -8;
         chunkX < 8;
         ++chunkX)
    {
        for (s32 chunkY = -8;
             chunkY < 8;
             ++chunkY)
        {
            if (chunkX == 1 && chunkY == 4)
            {
                int breakHere = 3;
            }
            
            chunk *c = getChunk(&app.overworld, chunkX, chunkY, 0);
            if (!c)
            {
                c = generateChunk(&app.overworld, chunkX, chunkY, 0);
            }
            
            v3 scale = v3{1,1,1};
            for (s32 z = 0;
                 z < CHUNK_SIZE.z;
                 ++z)
            {
                for (s32 y = 0;
                     y < CHUNK_SIZE.y;
                     ++y)
                {
                    for (s32 x = 0;
                         x < CHUNK_SIZE.x;
                         ++x)
                    {
                        u32 material = getVoxel(c, x, y, z);
                        
                        f32 compX = chunkX*scale.x*CHUNK_SIZE.x + x;
                        f32 compY = chunkY*scale.y*CHUNK_SIZE.y + y;
                        f32 compZ = 0*scale.z*CHUNK_SIZE.z + z;
                        
                        v3 p = 
                        {
                            compX*scale.x,
                            compZ*scale.z,
                            compY*scale.y
                        };
                        
                        hnSprite sprite = app.white;
                        v4 color = {};
                        if (material == 1)
                        {
                            sprite = app.dirt;
                        }
                        else if (material == 2)
                        {
                            sprite = app.stone;
                        }
                        else if (material == 3)
                        {
                            sprite = app.snow;
                        }
                        
                        if (material && hasSpaceAround(c, x, y, z))
                        {
                            hnPushCubeIndexed(&app.vb, &app.ib, sprite, p, scale, hnWHITE);
                        }
                    }
                }
            }
        }
    }
    
    hnUploadGpuBuffer(app.renderer, &app.vb);
    hnUploadGpuBuffer(app.renderer, &app.ib);
    
    
    while (!app.renderer->shouldClose)
    {
        hnPrepareFrame(app.renderer, 0);
        
#if 0
        app.vb.index = 0;
        app.ib.index = 0;
#endif
        
        hnMouse *mouse = &win32.input.mouse;
        hnKeyboard *key = &win32.input.keyboard;
        
        f32 dt = app.renderer->dt;
        
        f32 speed = 0.1f;
        
        if (key->w.down)
        {
            app.cam.pos += speed * app.cam.front;
        }
        
        if (key->s.down)
        {
            app.cam.pos -= speed * app.cam.front;
        }
        
        if (key->a.down)
        {
            app.cam.pos -= speed * normalize(cross(app.cam.front, app.cam.up));
        }
        
        if (key->d.down)
        {
            app.cam.pos += speed * normalize(cross(app.cam.front, app.cam.up));
        }
        
        if (key->space.down)
        {
            app.cam.pos += speed * app.cam.up;
        }
        
        if (key->shift.down)
        {
            app.cam.pos -= speed * app.cam.up;
        }
        
        
        v2 deltaP = mouse->pos - app.lastMouseP;
        app.lastMouseP = mouse->pos;
        
        
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
        
        v2 dim = v2{64,64} + sinf((f32)app.time)*v2{128,128};
        
        
        
        updateProjectionMatrix();
        hnExecuteRenderPass(app.renderer, &app.pass, &app.vb, &app.ib, true, hnDIMGRAY);
        
        hnPresentFrame(app.renderer, 0);
        
        app.time += app.renderer->dt;
    }
}