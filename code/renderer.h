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

#endif //RENDERER_H
