/* date = December 24th 2021 7:31 am */

#ifndef MATH_S32_H
#define MATH_S32_H

internal s32
clamp_s32(s32 value, s32 min, s32 max)
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

#endif //MATH_S32_H
