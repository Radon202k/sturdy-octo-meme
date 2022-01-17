/* date = January 1st 2022 11:28 am */

#ifndef RENDERER_H
#define RENDERER_H

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "stb_text.h"

#include <time.h>
time_t t;

internal void
renderer_init(memory_arena_t *platArena)
{
    opengl_init(&os.gl, 0, 0, 500, 500, "Minecraft clone", platArena);
    
    stbtt_initfont(platArena);
    
    os.window_is_open = 1;
    
    srand((unsigned) time(&t));
}

internal void
render_line(gl_vbuffer_t *buffer, v3 a, v3 b, v3 color)
{
    u32 indexBase = buffer->vertexCount;
    f32 lineVertices[] =
    {
        a.x, a.y, a.z, color.r, color.g, color.b,
        b.x, b.y, b.z, color.r, color.g, color.b,
    };
    u32 lineIndices[] =
    {
        indexBase + 0, indexBase + 1,
    };
    
    memcpy((u8 *)buffer->vertices + buffer->vertexCount * buffer->vertexSize, lineVertices, sizeof(f32) * 12);
    memcpy((u8 *)buffer->indices + buffer->indexCount * sizeof(u32), lineIndices, sizeof(u32) * 2);
    
    buffer->vertexCount += 2;
    buffer->indexCount += 2;
}

#endif //RENDERER_H
