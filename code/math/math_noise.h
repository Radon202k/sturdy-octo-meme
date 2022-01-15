/* date = December 30th 2021 9:17 pm */

#ifndef MATH_NOISE_H
#define MATH_NOISE_H

internal void
perlinlike_noise_seed(s32 count, f32 *output)
{
    // seed
    for (s32 i = 0;
         i < count;
         ++i)
    {
        output[i] = (float)rand() / (float)RAND_MAX;
    }
}

// IMPORTANT: count must always be a power of 2!!!
internal void
perlinlike_noise1d(s32 count, s32 octaves, f32 bias, f32 *seed, f32 *output)
{
    for (s32 x = 0;
         x < count;
         ++x)
    {
        f32 noise = 0.0f;
        f32 scale = 1.0f;
        f32 scaleAcc = 0.0f;
        
        for (s32 o = 0;
             o < octaves;
             ++o)
        {
            s32 pitch = count >> o;
            s32 sample1 = (x / pitch) * pitch;
            s32 sample2 = (sample1 + pitch) % count;
            
            f32 blend = (f32)(x - sample1) / (f32)pitch;
            f32 sample = (1.0f - blend) * seed[sample1] + blend * seed[sample2];
            noise += sample * scale;
            scaleAcc += scale;
            scale = scale / bias;
        }
        
        output[x] = noise / scaleAcc;
    }
}

// IMPORTANT: width and height must always be a power of 2!!!
internal void
perlinlike_noise2d(s32 width, s32 height, s32 octaves, f32 bias, 
                   f32 *seed, f32 *output)
{
    for (s32 x = 0;
         x < width;
         ++x)
    {
        for (s32 y = 0;
             y < height;
             ++y)
        {
            f32 noise = 0.0f;
            f32 scale = 1.0f;
            f32 scaleAcc = 0.0f;
            
            for (s32 o = 0;
                 o < octaves;
                 ++o)
            {
                s32 pitch = width >> o;
                s32 sampleX1 = (x / pitch) * pitch;
                s32 sampleY1 = (y / pitch) * pitch;
                
                s32 sampleX2 = (sampleX1 + pitch) % width;
                s32 sampleY2 = (sampleY1 + pitch) % width;
                
                f32 blendX = (f32)(x - sampleX1) / (f32)pitch;
                f32 blendY = (f32)(y - sampleY1) / (f32)pitch;
                
                f32 sampleT = (1.0f - blendX) * seed[sampleY1 * width + sampleX1] + blendX * seed[sampleY1 * width + sampleX2];
                f32 sampleB = (1.0f - blendX) * seed[sampleY2 * width + sampleX1] + blendX * seed[sampleY2 * width + sampleX2];
                
                noise += (blendY * (sampleB - sampleT) + sampleT) * scale;
                
                scaleAcc += scale;
                scale = scale / bias;
            }
            
            output[y * width + x] = noise / scaleAcc;
        }
    }
}

typedef struct opensimplex_noise_t
{
    f64 stretch2d;
    f64 squish2d;
    f64 norm2d;
    u64 defaultSeed;
    
    typeless_vector_t perm;
    typeless_vector_t gradients2d;
    
} opensimplex_noise_t;

internal opensimplex_noise_t 
make_opensimplex_noise()
{
    char data[] = 
    {
        +5, +2,   +2, +5,
        -5, +2,   -2, +5,
        +5, -2,   +2, -5,
        -5, -2,   -2, -5,
    };
    
    typeless_vector_t gradients2d = make_typeless_vector(16, sizeof(char));
    gradients2d.data = data;
    
    typeless_vector_t perm = make_typeless_vector(256, sizeof(int));
    
    opensimplex_noise_t result = 
    {
        .stretch2d = -0.211324865405187, // (1/sqrt(2+1)-1)/2
        .squish2d = 0.366025403784439, // (sqrt(2+1)-1)/2
        .norm2d = 47,
        .defaultSeed = 0,
        .perm = perm,
        .gradients2d = gradients2d,
    };
    
    return result;
}


internal f64
opensimplex_noise2d_extrapolate(opensimplex_noise_t *context, 
                                int xsb, int ysb, f64 dx, f64 dy)
{
    int *perm = ((int *)context->perm.data);
    char *grads2d = ((char *)context->gradients2d.data);
    int index = perm[(perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
    return grads2d[index] * dx + grads2d[index + 1] * dy;
}

internal f64
opensimplex_noise2d(opensimplex_noise_t *context, f64 x,  f64 y)
{
    // Place input coordinates onto grid.
    f64 stretchOffset = (x + y) * context->stretch2d;
    f64 xs = x + stretchOffset;
    f64 ys = y + stretchOffset;
    
    // Floor to get grid coordinates of rhombus (stretched square) super-cell origin.
    int xsb = (int)(floor(xs));
    int ysb = (int)(floor(ys));
    
    // Skew out to get actual coordinates of rhombus origin. We'll need these later.
    f64 squishOffset = (xsb + ysb) * context->squish2d;
    f64 xb = xsb + squishOffset;
    f64 yb = ysb + squishOffset;
    
    // Compute grid coordinates relative to rhombus origin.
    f64 xins = xs - xsb;
    f64 yins = ys - ysb;
    
    //Sum those together to get a value that determines which region we're in.
    f64 inSum = xins + yins;
    
    //Positions relative to origin point.
    f64 dx0 = x - xb;
    f64 dy0 = y - yb;
    
    //We'll be defining these inside the next block and using them afterwards.
    f64 dx_ext, dy_ext;
    int xsv_ext, ysv_ext;
    
    f64 value = 0;
    
    //Contribution (1,0)
    f64 dx1 = dx0 - 1 - context->squish2d;
    f64 dy1 = dy0 - 0 - context->squish2d;
    f64 attn1 = 2 - dx1 * dx1 - dy1 * dy1;
    if (attn1 > 0)
    {
        attn1 *= attn1;
        value += attn1 * attn1 * opensimplex_noise2d_extrapolate(context, xsb + 1, ysb + 0, dx1, dy1);
    }
    
    //Contribution (0,1)
    f64 dx2 = dx0 - 0 - context->squish2d;
    f64 dy2 = dy0 - 1 - context->squish2d;
    f64 attn2 = 2 - dx2 * dx2 - dy2 * dy2;
    if (attn2 > 0)
    {
        attn2 *= attn2;
        value += attn2 * attn2 * opensimplex_noise2d_extrapolate(context, xsb + 0, ysb + 1, dx2, dy2);
    }
    
    if (inSum <= 1)
    { //We're inside the triangle (2-Simplex) at (0,0)
        f64 zins = 1 - inSum;
        if (zins > xins || zins > yins)
        { //(0,0) is one of the closest two triangular vertices
            if (xins > yins)
            {
                xsv_ext = xsb + 1;
                ysv_ext = ysb - 1;
                dx_ext = dx0 - 1;
                dy_ext = dy0 + 1;
            }
            else
            {
                xsv_ext = xsb - 1;
                ysv_ext = ysb + 1;
                dx_ext = dx0 + 1;
                dy_ext = dy0 - 1;
            }
        }
        else
        { //(1,0) and (0,1) are the closest two vertices.
            xsv_ext = xsb + 1;
            ysv_ext = ysb + 1;
            dx_ext = dx0 - 1 - 2 * context->squish2d;
            dy_ext = dy0 - 1 - 2 * context->squish2d;
        }
    }
    else
    { //We're inside the triangle (2-Simplex) at (1,1)
        f64 zins = 2 - inSum;
        if (zins < xins || zins < yins)
        { //(0,0) is one of the closest two triangular vertices
            if (xins > yins)
            {
                xsv_ext = xsb + 2;
                ysv_ext = ysb + 0;
                dx_ext = dx0 - 2 - 2 * context->squish2d;
                dy_ext = dy0 + 0 - 2 * context->squish2d;
            }
            else
            {
                xsv_ext = xsb + 0;
                ysv_ext = ysb + 2;
                dx_ext = dx0 + 0 - 2 * context->squish2d;
                dy_ext = dy0 - 2 - 2 * context->squish2d;
            }
        }
        else
        { //(1,0) and (0,1) are the closest two vertices.
            dx_ext = dx0;
            dy_ext = dy0;
            xsv_ext = xsb;
            ysv_ext = ysb;
        }
        xsb += 1;
        ysb += 1;
        dx0 = dx0 - 1 - 2 * context->squish2d;
        dy0 = dy0 - 1 - 2 * context->squish2d;
    }
    
    //Contribution (0,0) or (1,1)
    f64 attn0 = 2 - dx0 * dx0 - dy0 * dy0;
    if (attn0 > 0)
    {
        attn0 *= attn0;
        value += attn0 * attn0 * opensimplex_noise2d_extrapolate(context, xsb, ysb, dx0, dy0);
    }
    
    //Extra Vertex
    f64 attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
    if (attn_ext > 0)
    {
        attn_ext *= attn_ext;
        value += attn_ext * attn_ext * opensimplex_noise2d_extrapolate(context, xsv_ext, ysv_ext, dx_ext, dy_ext);
    }
    
    return value / context->norm2d;
}

#endif //MATH_NOISE_H
