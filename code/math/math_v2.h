/* date = December 24th 2021 7:32 am */

#ifndef MATH_V2_H
#define MATH_V2_H

typedef union v2
{
    struct
    {
        float x;
        float y;
    };
    
    float data[2];
} v2;

inline v2
V2(f32 x, f32 y)
{
    v2 result = 
    {
        .x = x,
        .y = y,
    };
    
    return result;
}

inline v2
v2_add(v2 a, v2 b)
{
    v2 result = 
    {
        a.x + b.x,
        a.y + b.y,
    };
    
    return result;
}

inline v2
v2_sub(v2 a, v2 b)
{
    v2 result = 
    {
        a.x - b.x,
        a.y - b.y,
    };
    
    return result;
}

inline f32
v2_dot(v2 a, v2 b)
{
    f32 result = a.x * b.x + a.y * b.y;
    return result;
}

inline f32
v2_inner(v2 a, v2 b)
{
    return v2_dot(a, b);
}

inline f32
v2_scalar(v2 a, v2 b)
{
    return v2_dot(a, b);
}

inline v2
v2_neg(v2 a)
{
    v2 result = 
    {
        -a.x,
        -a.y,
    };
    
    return result;
}

inline v2
v2_perp(v2 a)
{
    // TODO: Make sure this is right
    v2 result = 
    {
        -a.y,
        a.x,
    };
    
    return result;
}

inline v2
v2_mul(v2 a, f32 k)
{
    v2 result = 
    {
        k * a.x,
        k * a.y,
    };
    
    return result;
}

#endif //MATH_V2_H
