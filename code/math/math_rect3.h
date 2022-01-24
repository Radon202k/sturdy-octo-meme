/* date = January 24th 2022 3:59 pm */

#ifndef MATH_RECT_H
#define MATH_RECT_H

typedef struct rect3
{
    v3 min;
    v3 max;
} rect3;

inline rect3
rect3_center_dim(v3 center, v3 dim)
{
    v3 halfDim = v3_mul(dim,0.5f);
    rect3 result = 
    {
        .min = v3_sub(center, halfDim),
        .max = v3_add(center, halfDim),
    };
    
    return result;
}


#endif //MATH_RECT_H
