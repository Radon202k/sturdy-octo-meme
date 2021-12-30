/* date = December 24th 2021 7:32 am */

#ifndef MATH_V3_H
#define MATH_V3_H

typedef union v3
{
    struct
    {
        float x;
        float y;
        float z;
    };
    
    struct
    {
        float r;
        float g;
        float b;
    };
    
    float data[3];
} v3;

inline v3
V3(f32 x, f32 y, f32 z)
{
    v3 result = 
    {
        .x = x,
        .y = y,
        .z = z,
    };
    
    return result;
}

inline v3
v3_add(v3 a, v3 b)
{
    v3 result = 
    {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
    
    return result;
}

inline v3
v3_sub(v3 a, v3 b)
{
    v3 result = 
    {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
    
    return result;
}

inline f32
v3_dot(v3 a, v3 b)
{
    f32 result = a.x * b.x + a.y * b.y + a.z * b.z;
    return result;
}

inline f32
v3_inner(v3 a, v3 b)
{
    return v3_dot(a, b);
}

inline f32
v3_scalar(v3 a, v3 b)
{
    return v3_dot(a, b);
}

inline v3
v3_neg(v3 a)
{
    v3 result = 
    {
        -a.x,
        -a.y,
        -a.z,
    };
    
    return result;
}

inline v3
v3_cross(v3 p, v3 q)
{
    v3 result = 
    {
        p.y*q.z - p.z*q.y,
        p.z*q.x - p.x*q.z,
        p.x*q.y - p.y*q.x,
    };
    
    return result;
}

inline v3
v3_mul(v3 a, f32 k)
{
    v3 result = 
    {
        k * a.x,
        k * a.y,
        k * a.z,
    };
    
    return result;
}

inline v3
v3_div(v3 a, f32 k)
{
    v3 result = v3_mul(a, 1.0f / k);
    return result;
}

inline f32
v3_lengthSq(v3 a)
{
    f32 result = v3_inner(a, a);
    return result;
}

inline f32
v3_length(v3 a)
{
    f32 result = sqrtf(v3_lengthSq(a));
    return result;
}

inline v3
v3_norm(v3 a)
{
    v3 result = v3_mul(a, 1.0f / v3_length(a));
    return result;
}

#endif //MATH_V3_H
