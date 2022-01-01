/* date = December 24th 2021 7:31 am */

#ifndef MATH_F32_H
#define MATH_F32_H

internal f32
sq_f32(f32 value)
{
    return value*value;
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

#endif //MATH_F32_H
