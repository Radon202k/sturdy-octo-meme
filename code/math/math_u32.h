/* date = December 24th 2021 7:29 am */

#ifndef MATH_SCALAR_H
#define MATH_SCALAR_H

internal u32
clamp_u32(u32 value, u32 min, u32 max)
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


#endif //MATH_SCALAR_H
