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


#endif //MATH_NOISE_H
