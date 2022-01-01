/* date = January 1st 2022 11:27 am */

#ifndef STB_TEXT_H
#define STB_TEXT_H

unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs
GLuint ftex;

void stbtt_initfont(memory_arena *arena)
{
    fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
    stbtt_BakeFontBitmap(ttf_buffer,0, 32.0, temp_bitmap,512,512, 32,96, cdata); // no guarantee this fits!
    
    u32 *output = push_array(arena, 512*512, u32, 4);
    
    for (u32 y = 0;
         y < 512;
         ++y)
    {
        for (u32 x = 0;
             x < 512;
             ++x)
        {
            u8 alpha = temp_bitmap[y * 512 + x];
            
            if (alpha == 255)
            {
                int breakHere = 2;
            }
            
            output[y * 512 + x] = (alpha << 24) | (alpha << 16) | (alpha << 8) | (alpha << 0);
        }
    }
    
    opengl_make_texture(&ftex, 512, 512, output, GL_RGBA, GL_LINEAR);
}

void stbtt_print(memory_arena *arena, opengl_vertexbuffer *b, float x, float y, char *text)
{
    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(cdata, 512,512, *text-32, &x,&y,&q,1);//1=opengl & d3d10+,0=d3d9
            
            opengl_mesh_indexed quad = opengl_make_quad_indexed(arena, V2(q.x0,q.y0), V2(q.x1,q.y1), 
                                                                V2(q.s0,q.t0), V2(q.s1,q.t1), b->vertexCount);
            
            memcpy(b->vertices + b->vertexCount, quad.vertices, sizeof(opengl_vertex) * quad.vertexCount);
            memcpy(b->indices + b->indexCount, quad.indices, sizeof(u32) * quad.indexCount);
            
            b->vertexCount += quad.vertexCount;
            b->indexCount += quad.indexCount;
        }
        ++text;
    }
}

#endif //STB_TEXT_H
