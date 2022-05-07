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

#define CHUNK_SIZE v3{16,16,64}

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
    chunk *freeNext;
    chunk *loadedNext;
    chunk *nextInHash;
};

struct noise_hash_node
{
    s32 x;
    s32 y;
    f32 value;
    noise_hash_node *nextInHash;
    noise_hash_node *freeNext;
};

struct voxel_map
{
    hnMandala *permanent;
    chunk *hash[2048];
    
    noise_hash_node *noiseHash[4096];
    noise_hash_node *noiseHashFreeFirst;
    
    chunk *freeFirst;
    chunk *loadedFirst;
};

inline u32
getNoiseHashIndex(voxel_map *map, s32 x, s32 y)
{
    assert(arrayCount(map->noiseHash) % 2 == 0);
    // TODO: Better hash function !!
    u32 hashIndex = (u32)(x * 17 + y * 39);
    return hashIndex % arrayCount(map->noiseHash);
}

inline noise_hash_node *
makeNoiseHashNode(voxel_map *map, s32 x, s32 y)
{
    if (!map->noiseHashFreeFirst)
    {
        map->noiseHashFreeFirst = hnPushStruct(map->permanent, noise_hash_node);
    }
    
    noise_hash_node *result = map->noiseHashFreeFirst;
    map->noiseHashFreeFirst = map->noiseHashFreeFirst->freeNext;
    
    result->x = x;
    result->y = y;
    result->value = 0;
    
    return result;
}

internal noise_hash_node *
getNoise(voxel_map *map, s32 x, s32 y)
{
    u32 hashIndex = getNoiseHashIndex(map,x,y); 
    
    noise_hash_node *result = map->noiseHash[hashIndex];
    
    if (result)
    {
        noise_hash_node *lastValid = result;
        while (result && !(result->x == x && result->y == y))
        {
            lastValid = result;
            result = result->nextInHash;
        }
        
        if (result)
        {
            assert((result->x == x && result->y == y));
        }
        else
        {
            // didnt find one, has to make it
            result = makeNoiseHashNode(map, x, y);
        }
    }
    else
    {
        result = makeNoiseHashNode(map, x, y);
    }
    
    return result;
}

internal void
setNoise(voxel_map *map, s32 x, s32 y, f32 value)
{
    u32 hashIndex = getNoiseHashIndex(map,x,y); 
    
    noise_hash_node *node = getNoise(map, x, y);
    assert(node);
    
    node->value = value;
}

inline u32
getChunkHashIndex(voxel_map *map, s32 x, s32 y, s32 z)
{
    assert(arrayCount(map->hash) % 2 == 0);
    // TODO: Better hash function !!
    u32 hashIndex = (u32)(x * 17 + y * 39 + z * 121);
    return hashIndex % arrayCount(map->hash);
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

internal void
freeChunk(voxel_map *map, chunk *c)
{
    c->freeNext = map->freeFirst;
    map->freeFirst = c;
}

internal void
freeLoadedChunksAtOnce(voxel_map *map)
{
    chunk *c = map->loadedFirst;
    while (c)
    {
        freeChunk(map, c);
        c = c->loadedNext;
    }
    map->loadedFirst = 0;
    
}

internal chunk *
generateChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    if (!map->freeFirst)
    {
        map->freeFirst = hnPushStruct(map->permanent, chunk);
    }
    
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    map->hash[hashIndex] = map->freeFirst;
    map->freeFirst = map->freeFirst->freeNext;
    
    chunk *result = map->hash[hashIndex];
    
    result->x = x;
    result->y = y;
    result->z = z;
    
    s32 voxelCount = (s32)(CHUNK_SIZE.x*CHUNK_SIZE.y*CHUNK_SIZE.z);
    result->voxels = hnPushArray(map->permanent, voxelCount, u32);
    
    f32 maxHeight = 64;
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
                
                noise_hash_node *height = getNoise(map, (s32)compX, (s32)compY);
                
                if (compZ <= height->value)
                {
                    if (compZ > maxHeight/1.5f)
                    {
                        cubeMaterial = 3;
                    }
                    else if (compZ > maxHeight/2)
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

inline chunk *
getChunk(voxel_map *map, s32 x, s32 y, s32 z)
{
    u32 hashIndex = getChunkHashIndex(map, x, y, z);
    chunk *result = map->hash[hashIndex];
    
    if (result)
    {
        chunk *lastValidChunkInHash = result;
        while (result && ((result->x == x && result->y == y && result->z == z) == false))
        {
            lastValidChunkInHash = result;
            result = result->nextInHash;
        }
        
        if (result)
        {
            assert((result->x == x && result->y == y && result->z == z));
        }
        else
        {
            // Didn't find the chunk, need to add a new chunk in the hash chain
            chunk *newChunk = generateChunk(map, x, y, 0);
            lastValidChunkInHash->nextInHash = newChunk;
            result = newChunk;
        }
    }
    else
    {
        result = generateChunk(map, x, y, 0);
        result->loadedNext = map->loadedFirst;
        map->loadedFirst = result;
    }
    
    return result;
}

internal void
generateNoise2DAroundP(voxel_map *map, v3 center, f32 dist, f32 maxHeight)
{
    s32 centerChunkX = (s32)(center.x / CHUNK_SIZE.x);
    s32 centerChunkY = (s32)(center.z / CHUNK_SIZE.y);
    
    // TODO: Z Layers!
    s32 chunkZ = 0;
    for (s32 chunkX = centerChunkX - (s32)dist;
         chunkX < centerChunkX + dist;
         ++chunkX)
    {
        for (s32 chunkY = centerChunkY - (s32)dist;
             chunkY < centerChunkY + dist;
             ++chunkY)
        {
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
                        
                        f32 compX = (f32)(chunkX*CHUNK_SIZE.x + voxelX);
                        f32 compY = (f32)(chunkY*CHUNK_SIZE.y + voxelY);
                        f32 compZ = (f32)(voxelZ);
                        
                        f32 height = floorf(maxHeight*(f32)hnPerlin2D(compX,compY,0.05f,6));
                        setNoise(map, (s32)compX, (s32)compY, height);
                    }
                }
            }
        }
    }
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

struct render_batch
{
    hnGpuBuffer vb;
    hnGpuBuffer ib;
    hnRenderPass pass;
};

struct globals
{
    hnMandala permanent;
    hnMandala temporary;
    
    hnRenderer *renderer;
    hnAudio *audio;
    
    hnShader shader3d;
    hnShader shader2d;
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
    
    render_batch cubes;
    render_batch ortho2d;
    
    hnRandomSeries entropy;
    
    f64 time;
    
    f32 dtHistory[200];
    u32 dtHistoryIndex;
    
    voxel_map overworld;
    camera cam;
    
    f32 viewDist;
    v3 currentCenter; // For chunk loading
    v3 maxDistFromCurrentCenter;
    
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
    
    memcpy((u8 *)app.cubes.pass.uniforms[0].data, proj.forward.e, sizeof(proj.forward));
    memcpy((u8 *)app.cubes.pass.uniforms[1].data, view.e, sizeof(view));
    
    //
    mat4x4_inv ortho = hnOrthographicProjection(win32.clientDim);
    memcpy((u8 *)app.ortho2d.pass.uniforms[0].data, ortho.forward.e, sizeof(ortho.forward));
}

internal void
hnResizeCallback(void)
{
    app.cubes.pass.scissor = hnRectMinMax({0,0},win32.clientDim);
    updateProjectionMatrix();
}

struct vertex2d { v3 pos; v3 uv; v4 col; };
struct vertex3d { v3 pos; v3 uv; v3 nor; v4 col; };

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


internal void
generateChunksAroundP(voxel_map *map, v3 center, f32 dist)
{
    app.cubes.vb.index = 0;
    app.cubes.ib.index = 0;
    
    s32 centerChunkX = (s32)(center.x / CHUNK_SIZE.x);
    s32 centerChunkY = (s32)(center.z / CHUNK_SIZE.y);
    
    freeLoadedChunksAtOnce(map);
    
    // TODO: Z Layers!
    for (s32 chunkX = centerChunkX - (s32)dist;
         chunkX < centerChunkX + dist;
         ++chunkX)
    {
        for (s32 chunkY = centerChunkY - (s32)dist;
             chunkY < centerChunkY + dist;
             ++chunkY)
        {
            chunk *c = getChunk(&app.overworld, chunkX, chunkY, 0);
            
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
                        f32 compZ = (f32)z;
                        
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
                            hnPushCubeIndexed(&app.cubes.vb, &app.cubes.ib, sprite, p, scale, hnWHITE);
                        }
                    }
                }
            }
        }
    }
    
    
    hnUploadGpuBuffer(app.renderer, &app.cubes.vb);
    hnUploadGpuBuffer(app.renderer, &app.cubes.ib);
    
}

internal void
drawDtHistoryGraph(v2 p, f32 height)
{
    f32 maxDt = 0;
    for (u32 dtIndex = 0;
         dtIndex < arrayCount(app.dtHistory);
         ++dtIndex)
    {
        f32 dtValue = app.dtHistory[dtIndex];
        if (dtValue > maxDt)
        {
            maxDt = dtValue;
        }
    }
    
    for (u32 dtIndex = 0;
         dtIndex < arrayCount(app.dtHistory);
         ++dtIndex)
    {
        f32 dtValue = app.dtHistory[dtIndex];
        
        f32 relDt = (dtValue / maxDt);
        
        f32 barHeight = relDt * height;
        f32 barWidth = 2;
        f32 margin = 1;
        
        v2 barDim = 
        {
            barWidth,
            barHeight
        };
        
        v3 barP =
        {
            dtIndex * (barWidth + margin),
            height - barHeight,
        };
        
        v4 color = hnGOLD;
        if (maxDt > 0.018f)
        {
            if (relDt > 0.5f)
            {
                color = hnCRIMSON;
                
                char dtLabel[256] = {};
                hnMakeFloatLabel(dtLabel,sizeof(dtLabel),dtValue);
                
                u32 debugLine = 0;
                
                hnPushStringIndexed(&app.font, &app.textureArray, &app.ortho2d.vb, &app.ortho2d.ib,
                                    dtLabel, toV3(p,0) + barP, hnCRIMSON, 0, false, 1);
                
            }
            else if (relDt > 0.25f)
            {
                color = hnYELLOW;
            }
        }
        
        hnPushSpriteIndexed(&app.ortho2d.vb, &app.ortho2d.ib, app.white, toV3(p,0) + barP, barDim, color, 0, false);
    }
    
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
    
    app.cubes.vb = hnMakeVertexBuffer(app.renderer, megabytes(512), sizeof(f32)*13);
    app.cubes.ib = hnMakeIndexBuffer(app.renderer, megabytes(128));
    
    hnSetInputLayout(&app.cubes.vb, 0, GL_FLOAT, 3, offsetof(vertex3d, pos));
    hnSetInputLayout(&app.cubes.vb, 1, GL_FLOAT, 3, offsetof(vertex3d, uv));
    hnSetInputLayout(&app.cubes.vb, 2, GL_FLOAT, 3, offsetof(vertex3d, nor));
    hnSetInputLayout(&app.cubes.vb, 3, GL_FLOAT, 4, offsetof(vertex3d, col));
    
    app.ortho2d.vb = hnMakeVertexBuffer(app.renderer, megabytes(4), sizeof(f32)*10);
    app.ortho2d.ib = hnMakeIndexBuffer(app.renderer, megabytes(1));
    
    hnSetInputLayout(&app.ortho2d.vb, 0, GL_FLOAT, 3, offsetof(vertex2d, pos));
    hnSetInputLayout(&app.ortho2d.vb, 1, GL_FLOAT, 3, offsetof(vertex2d, uv));
    hnSetInputLayout(&app.ortho2d.vb, 2, GL_FLOAT, 4, offsetof(vertex2d, col));
    
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
        
        app.shader3d = hnMakeShader(app.renderer, vertexShader, fragShader);
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
        
        app.shader2d = hnMakeShader(app.renderer, vertexShader, fragShader);
    }
    
    app.cubes.pass = {};
    app.cubes.pass.primitive = hnTRIANGLES;
    app.cubes.pass.shader = &app.shader3d;
    app.cubes.pass.target = app.renderer->defaultTarget;
    app.cubes.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    app.cubes.pass.blendEnabled = true;
    app.cubes.pass.blendType = hnPREMULTIPLIED_ALPHA;
    app.cubes.pass.depthTestEnabled = true;
    app.cubes.pass.depthTestType = hnLESS_EQUAL;
    
    app.cubes.pass.uniformCount = 2;
    app.cubes.pass.uniforms = hnPushArray(&app.permanent, app.cubes.pass.uniformCount, hnGpuBuffer);
    
    app.cubes.pass.uniforms[0] = hnMakeUniformBuffer(app.renderer, app.shader3d.vShader, 0, sizeof(f32)*4*4);
    app.cubes.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    app.cubes.pass.uniforms[1] = hnMakeUniformBuffer(app.renderer, app.shader3d.vShader, 1, sizeof(f32)*4*4);
    app.cubes.pass.uniforms[1].type = hnUNIFORM_MAT4FV;
    
    //
    
    app.ortho2d.pass = {};
    app.ortho2d.pass.primitive = hnTRIANGLES;
    app.ortho2d.pass.shader = &app.shader2d;
    app.ortho2d.pass.target = app.renderer->defaultTarget;
    app.ortho2d.pass.scissor = hnRectMinMax({0,0}, win32.clientDim);
    app.ortho2d.pass.blendEnabled = true;
    app.ortho2d.pass.blendType = hnPREMULTIPLIED_ALPHA;
    app.ortho2d.pass.depthTestEnabled = false;
    app.ortho2d.pass.depthTestType = hnLESS_EQUAL;
    
    app.ortho2d.pass.uniformCount = 1;
    app.ortho2d.pass.uniforms = hnPushArray(&app.permanent, app.ortho2d.pass.uniformCount, hnGpuBuffer);
    
    app.ortho2d.pass.uniforms[0] = hnMakeUniformBuffer(app.renderer, app.shader2d.vShader, 0, sizeof(f32)*4*4);
    app.ortho2d.pass.uniforms[0].type = hnUNIFORM_MAT4FV;
    
    //
    
    app.textureArray = hnMakeTexture2DArray(app.renderer, 1024, 1024, 32, hnNEAREST);
    app.font = hnInitFont(app.renderer, &app.textureArray, &app.fontSprite, "C:\\Windows\\Fonts\\arial.ttf",16);
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.white, "images/white.png", hnNEAREST);
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.circle, "images/circle.png", hnNEAREST);
    hnLoadTexture2DIntoArray(app.renderer, &app.textureArray, &app.atlas, "images/atlas.png", hnNEAREST);
    glGenerateTextureMipmap(app.textureArray.handle);
    
    hnEndTempMemory(tempMem);
    
    //
    
    app.cubes.pass.textureUnit = 0;
    app.cubes.pass.textureHandle = app.textureArray.handle;
    
    app.ortho2d.pass.textureUnit = 0;
    app.ortho2d.pass.textureHandle = app.textureArray.handle;
    
    //
    
    app.dirt = hnMakeSprite(app.atlas,16,{2,0},{3,1});
    app.stone = hnMakeSprite(app.atlas,16,{1,0},{2,1});
    app.snow = hnMakeSprite(app.atlas,16,{2,4},{3,5});
    
    app.cam.yaw = -90.0f;
    app.cam.pitch = 0;
    
    app.cam.pos = {0,30,3};
    app.cam.front = {0,0,-1};
    app.cam.up = {0,1,0};
    
    app.viewDist = 2;
    app.currentCenter = app.cam.pos;
    app.maxDistFromCurrentCenter = CHUNK_SIZE;
    
    app.entropy = hnRandomSeed(123);
    
    app.lastMouseP = win32.input.mouse.pos;
    
    app.overworld.permanent = &app.permanent;
    memset(app.overworld.hash, 0, sizeof(chunk *) * arrayCount(app.overworld.hash));
    
    
    generateNoise2DAroundP(&app.overworld, app.cam.pos, app.viewDist, 64);
    
    generateChunksAroundP(&app.overworld, app.cam.pos, app.viewDist);
    
    SetCapture(win32.window);
    ShowCursor(0);
    
    RECT clipRect;
    GetClientRect(win32.window, &clipRect);
    ClientToScreen(win32.window, (POINT *)&clipRect.left);
    ClientToScreen(win32.window, (POINT *)&clipRect.right);
    ClipCursor(&clipRect);
    
    while (!app.renderer->shouldClose)
    {
        hnPrepareFrame(app.renderer, 0);
        
        hnMouse *mouse = &win32.input.mouse;
        hnKeyboard *key = &win32.input.keyboard;
        
        f32 dt = app.renderer->dt;
        
        f32 speed = 1.0f;
        
        //
        
        app.ortho2d.ib.index = 0;
        app.ortho2d.vb.index = 0;
        
        //
        
        if (key->escape.down)
        {
            app.renderer->shouldClose = true;
        }
        
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
        
        v2 dim = v2{64,64} + sinf((f32)app.time)*v2{128,128};
        
        f32 xDiff = app.cam.pos.x - app.currentCenter.x;
        f32 zDiff = app.cam.pos.z - app.currentCenter.z;
        
        if (absolute(xDiff) > app.maxDistFromCurrentCenter.x ||
            absolute(zDiff) > app.maxDistFromCurrentCenter.z)
        {
            app.currentCenter = app.cam.pos;
            
            generateChunksAroundP(&app.overworld, app.cam.pos, app.viewDist);
        }
        
        char dtLabel[256] = {};
        hnMakeFloatLabel(dtLabel,sizeof(dtLabel),dt,"Delta time");
        
        u32 debugLine = 0;
        
        hnPushStringIndexed(&app.font, &app.textureArray, &app.ortho2d.vb, &app.ortho2d.ib,
                            dtLabel, {0,(f32)(++debugLine)*16}, hnGOLD, 0, false, 1);
        
        char cameraPosLabel[256] = {};
        hnMakeV3Label(cameraPosLabel,sizeof(cameraPosLabel),app.cam.pos,"Camera world pos");
        
        hnPushStringIndexed(&app.font, &app.textureArray, &app.ortho2d.vb, &app.ortho2d.ib,
                            cameraPosLabel, {0,(f32)(++debugLine)*16}, hnGOLD, 0, false, 1);
        
        drawDtHistoryGraph({0,win32.clientDim.y-100},100);
        
        hnUploadGpuBuffer(app.renderer, &app.ortho2d.vb);
        hnUploadGpuBuffer(app.renderer, &app.ortho2d.ib);
        
        updateProjectionMatrix();
        hnExecuteRenderPass(app.renderer, &app.cubes.pass, &app.cubes.vb, &app.cubes.ib, true, hnDIMGRAY);
        hnExecuteRenderPass(app.renderer, &app.ortho2d.pass, &app.ortho2d.vb, &app.ortho2d.ib, false, {});
        
        hnPresentFrame(app.renderer, 0);
        
        app.dtHistory[app.dtHistoryIndex++] = app.renderer->dt;
        if (app.dtHistoryIndex == arrayCount(app.dtHistory))
        {
            app.dtHistoryIndex = 0;
        }
        
        app.time += app.renderer->dt;
    }
    
    ReleaseCapture();
}