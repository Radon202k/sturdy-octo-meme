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

typedef struct line_vertex
{
    v3 pos;
    u32 color;
} line_vertex;

internal void
draw_line(gl_vbuffer_t *buffer, v3 a, v3 b, v4 color)
{
    u32 cr = (u32)(color.r*255.0f);
    u32 cg = (u32)(color.g*255.0f);
    u32 cb = (u32)(color.b*255.0f);
    u32 ca = (u32)(color.a*255.0f);
    
    u32 c = ((cr << 0) | (cg << 8) | (cb << 16) | (ca << 24));
    
    u32 indexBase = buffer->vertexCount;
    line_vertex lineVertices[] =
    {
        {{a.x,a.y,a.z}, c},
        {{b.x,b.y,b.z}, c},
    };
    u32 lineIndices[] =
    {
        indexBase + 0, indexBase + 1,
    };
    
    memcpy((u8 *)buffer->vertices + buffer->vertexCount * buffer->vertexSize, lineVertices, sizeof(lineVertices));
    memcpy((u8 *)buffer->indices + buffer->indexCount * sizeof(u32), lineIndices, sizeof(lineIndices));
    
    buffer->vertexCount += 2;
    buffer->indexCount += 2;
}


internal void
draw_bounding_box(gl_vbuffer_t *buffer, rect3 box)
{
    // X
    draw_line(buffer, 
              V3(box.min.x,box.min.y,box.min.z),
              V3(box.max.x,box.min.y,box.min.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.min.x,box.max.y,box.min.z),
              V3(box.max.x,box.max.y,box.min.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.min.x,box.min.y,box.max.z),
              V3(box.max.x,box.min.y,box.max.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.min.x,box.max.y,box.max.z),
              V3(box.max.x,box.max.y,box.max.z),
              V4(0,0,0,0.3f));
    
    // Y
    draw_line(buffer, 
              V3(box.min.x,box.min.y,box.min.z),
              V3(box.min.x,box.max.y,box.min.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.max.x,box.min.y,box.min.z),
              V3(box.max.x,box.max.y,box.min.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.min.x,box.min.y,box.max.z),
              V3(box.min.x,box.max.y,box.max.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.max.x,box.min.y,box.max.z),
              V3(box.max.x,box.max.y,box.max.z),
              V4(0,0,0,0.3f));
    
    
    // Z
    draw_line(buffer, 
              V3(box.min.x,box.min.y,box.min.z),
              V3(box.min.x,box.min.y,box.max.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.max.x,box.min.y,box.min.z),
              V3(box.max.x,box.min.y,box.max.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.min.x,box.max.y,box.min.z),
              V3(box.min.x,box.max.y,box.max.z),
              V4(0,0,0,0.3f));
    
    draw_line(buffer, 
              V3(box.max.x,box.max.y,box.min.z),
              V3(box.max.x,box.max.y,box.max.z),
              V4(0,0,0,0.3f));
}


#endif //RENDERER_H
