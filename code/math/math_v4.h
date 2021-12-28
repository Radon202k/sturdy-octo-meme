/* date = December 28th 2021 4:08 pm */

#ifndef MATH_V4_H
#define MATH_V4_H

typedef union v4
{
    struct
    {
        float x;
        float y;
        float z;
        float w;
    };
    
    struct
    {
        float r;
        float g;
        float b;
        float a;
    };
    
    float data[4];
} v4;


inline v4
V4(f32 x, f32 y, f32 z, f32 w)
{
    v4 result = 
    {
        .x = x,
        .y = y,
        .z = z,
        .w = w,
    };
    
    return result;
}

inline v4
v4_add(v4 a, v4 b)
{
    v4 result = 
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
        a.w + b.w,
    };
    
    return result;
}

inline v4
v4_sub(v4 a, v4 b)
{
    v4 result = 
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
        a.w - b.w,
    };
    
    return result;
}

inline f32
v4_dot(v4 a, v4 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    return result;
}


#endif //MATH_V4_H
