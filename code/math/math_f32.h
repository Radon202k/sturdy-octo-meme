/* date = December 24th 2021 7:31 am */

#ifndef MATH_F32_H
#define MATH_F32_H

internal f32
sq_f32(f32 value)
{
    return value*value;
}

internal f32
floor_f32(f32 value)
{
    f32 result = floorf(value);
    return result;
}

internal f32
clamp_f32(f32 value, f32 min, f32 max)
{
    if (value < min)
    {
        return min;
    }
    else if (value > max)
    {
        return max;
    }
    else
    {
        return value;
    }
}

internal f32
lerp_f32(f32 a, f32 t, f32 b)
{
    f32 result = a * (1-t) + b * t;
    return result;
}

internal f32
image_pixel_as_float(image_t *image, u32 x, u32 y)
{
    u32 position = (x + image->width * y);
    f32 result = image->pixels[position] / 255.0f;
    return result;
}

internal f32
bilinear_sample_f32(image_t *image, f32 xf, f32 yf)
{
    f32 wf = (f32)image->width;
    f32 hf = (f32)image->height;
    
    f32 x1 = floor_f32(xf * image->width);
    f32 y1 = floor_f32(yf * image->height);
    f32 x2 = clamp_f32(x1 + 1, 0, wf);
    f32 y2 = clamp_f32(y1 + 1, 0, hf);
    
    f32 xp = xf * wf - x1;
    f32 yp = yf * hf - y1;
    
    f32 p11 = image_pixel_as_float(image, (u32)x1, (u32)y1);
    f32 p21 = image_pixel_as_float(image, (u32)x2, (u32)y1);
    f32 p12 = image_pixel_as_float(image, (u32)x1, (u32)y2);
    f32 p22 = image_pixel_as_float(image, (u32)x2, (u32)y2);
    
    f32 px1 = lerp_f32(xp, p11, p21);
    f32 px2 = lerp_f32(xp, p12, p22);
    
    f32 result = lerp_f32(yp, px1, px2);
    return result;
}

#endif //MATH_F32_H
