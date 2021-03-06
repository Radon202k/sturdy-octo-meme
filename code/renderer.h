/* date = May 7th 2022 9:57 pm */

#ifndef RENDERER_H
#define RENDERER_H

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

struct vertex2d { v3 pos; v3 uv; u32 col; };
struct vertex3d_cubes { v3 pos; v3 uv; v3 nor; };
struct vertex3d_lines { v3 pos; u32 col; };

struct camera
{
    v3 pos;
    v3 front;
    v3 up;
    
    f32 yaw;
    f32 pitch;
};

#define CHUNK_HASH_TABLE_SIZE 4096

struct mcRenderer
{
    hnRenderer *backend;
    
    hnShader shader3d;
    hnShader lineShader3d;;
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
    
    hnRenderPass cubes;
    hnRenderPass lines3d;
    
    hnRenderPass ortho2d;
    hnGpuBuffer *ortho2dVb;
    hnGpuBuffer *ortho2dIb;
    
    hnRenderPass lines;
    hnGpuBuffer *linesVb;
    hnGpuBuffer *linesIb;
};

#endif //RENDERER_H
