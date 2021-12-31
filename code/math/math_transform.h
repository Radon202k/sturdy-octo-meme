/* date = December 31st 2021 3:48 pm */

#ifndef MATH_TRANSFORM_H
#define MATH_TRANSFORM_H

typedef struct transform_t
{
    v3 translation;
    v3 rotation;
    v3 scale;
} transform_t;

inline transform_t
transform_get_default(void)
{
    transform_t result = 
    {
        .translation = {0,0,0},
        .rotation = {0,0,0},
        .scale = {1,1,1},
    };
    return result;
}

internal mat4
transform_make_object_matrix(transform_t t)
{
    mat4 result = mat4_translation(t.translation);
    result = mat4_mul(result, mat4_xrotation(t.rotation.x));
    result = mat4_mul(result, mat4_yrotation(t.rotation.y));
    result = mat4_mul(result, mat4_zrotation(t.rotation.z));
    result = mat4_mul(result, mat4_scale(t.scale));
    return result;
}


#endif //MATH_TRANSFORM_H
