/* date = December 24th 2021 7:42 am */

#ifndef MATH_MAT4_H
#define MATH_MAT4_H

typedef union mat4
{
    float data[16];
    struct 
    {
        f32 m00;
        f32 m01;
        f32 m02;
        f32 m03;
        f32 m10;
        f32 m11;
        f32 m12;
        f32 m13;
        f32 m20;
        f32 m21;
        f32 m22;
        f32 m23;
        f32 m30;
        f32 m31;
        f32 m32;
        f32 m33;
    };
} mat4;

typedef struct mat4_inv
{
    mat4 forward;
    mat4 inverse;
} mat4_inv;

inline mat4
mat4_identity(f32 diagonal)
{
    mat4 result = 
    {
        diagonal, 0, 0, 0,
        0, diagonal, 0, 0,
        0, 0, diagonal, 0,
        0, 0, 0, diagonal,
    };
    
    return result;
}

inline v4
mat4_row0(mat4 matrix)
{
    v4 result = {matrix.data[0], matrix.data[1], matrix.data[2], matrix.data[3]};
    return result;
}

inline v4
mat4_row1(mat4 matrix)
{
    v4 result = {matrix.data[4], matrix.data[5], matrix.data[6], matrix.data[7]};
    return result;
}

inline v4
mat4_row2(mat4 matrix)
{
    v4 result = {matrix.data[8], matrix.data[9], matrix.data[10], matrix.data[11]};
    return result;
}

inline v4
mat4_row3(mat4 matrix)
{
    v4 result = {matrix.data[12], matrix.data[13], matrix.data[14], matrix.data[15]};
    return result;
}

inline v4
mat4_col0(mat4 matrix)
{
    v4 result = {matrix.data[0], matrix.data[4], matrix.data[8], matrix.data[12]};
    return result;
}

inline v4
mat4_col1(mat4 matrix)
{
    v4 result = {matrix.data[1], matrix.data[5], matrix.data[9], matrix.data[13]};
    return result;
}

inline v4
mat4_col2(mat4 matrix)
{
    v4 result = {matrix.data[2], matrix.data[6], matrix.data[10], matrix.data[14]};
    return result;
}

inline v4
mat4_col3(mat4 matrix)
{
    v4 result = {matrix.data[3], matrix.data[7], matrix.data[11], matrix.data[15]};
    return result;
}

internal mat4
mat4_mul(mat4 a, mat4 b)
{
    mat4 result = 
    {
        v4_dot(mat4_row0(a),mat4_col0(b)), v4_dot(mat4_row0(a),mat4_col1(b)), v4_dot(mat4_row0(a),mat4_col2(b)), v4_dot(mat4_row0(a),mat4_col3(b)),
        v4_dot(mat4_row1(a),mat4_col0(b)), v4_dot(mat4_row1(a),mat4_col1(b)), v4_dot(mat4_row1(a),mat4_col2(b)), v4_dot(mat4_row1(a),mat4_col3(b)),
        v4_dot(mat4_row2(a),mat4_col0(b)), v4_dot(mat4_row2(a),mat4_col1(b)), v4_dot(mat4_row2(a),mat4_col2(b)), v4_dot(mat4_row2(a),mat4_col3(b)),
        v4_dot(mat4_row3(a),mat4_col0(b)), v4_dot(mat4_row3(a),mat4_col1(b)), v4_dot(mat4_row3(a),mat4_col2(b)), v4_dot(mat4_row3(a),mat4_col3(b)),
    };
    
    return result;
}

internal mat4
mat4_mul_v4(mat4 a, v4 b)
{
    mat4 result = a;
    
    return result;
}

internal mat4_inv
mat4_perspective(f32 ar, f32 fov, f32 zFar, f32 zNear)
{
    /*
   ar: Aspect Ratio, must be height / width
    fov: Field of view, must be in radians
*/
    
    f32 f = 1.0f / tanf(0.5f * fov);
    f32 l = zFar / (zFar - zNear);
    
    f32 a = ar * f;
    f32 b = f;
    f32 c = l;
    f32 d = -l * zNear;
    
    mat4_inv result = 
    {
        .forward = 
        {
            a, 0, 0, 0,
            0, b, 0, 0,
            0, 0, c, 1,
            0, 0, d, 1,
        },
        
        .inverse =
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        },
    };
    
    return result;
}

internal mat4
mat4_camera_3x3(v3 x, v3 y, v3 z, v3 p)
{
#if 1
    mat4 result = 
    {
        x.x, y.x, z.x, 0,
        x.y, y.y, z.y, 0,
        x.z, y.z, z.z, 0,
        p.x, p.y, p.z, 1,
    };
#else
    mat4 result = 
    {
        x.x, x.y, x.z, p.x,
        y.x, y.y, y.z, p.y,
        z.x, z.y, z.z, p.z,
        0, 0, 0, 1,
    };
#endif
    
    return result;
}

internal mat4
mat4_invert(mat4 m)
{
    f32 A2323 = m.m22 * m.m33 - m.m23 * m.m32;
    f32 A1323 = m.m21 * m.m33 - m.m23 * m.m31;
    f32 A1223 = m.m21 * m.m32 - m.m22 * m.m31;
    f32 A0323 = m.m20 * m.m33 - m.m23 * m.m30;
    f32 A0223 = m.m20 * m.m32 - m.m22 * m.m30;
    f32 A0123 = m.m20 * m.m31 - m.m21 * m.m30;
    f32 A2313 = m.m12 * m.m33 - m.m13 * m.m32;
    f32 A1313 = m.m11 * m.m33 - m.m13 * m.m31;
    f32 A1213 = m.m11 * m.m32 - m.m12 * m.m31;
    f32 A2312 = m.m12 * m.m23 - m.m13 * m.m22;
    f32 A1312 = m.m11 * m.m23 - m.m13 * m.m21;
    f32 A1212 = m.m11 * m.m22 - m.m12 * m.m21;
    f32 A0313 = m.m10 * m.m33 - m.m13 * m.m30;
    f32 A0213 = m.m10 * m.m32 - m.m12 * m.m30;
    f32 A0312 = m.m10 * m.m23 - m.m13 * m.m20;
    f32 A0212 = m.m10 * m.m22 - m.m12 * m.m20;
    f32 A0113 = m.m10 * m.m31 - m.m11 * m.m30;
    f32 A0112 = m.m10 * m.m21 - m.m11 * m.m20;
    
    f32 det = 
        + m.m00 * (m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223) 
        - m.m01 * (m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223) 
        + m.m02 * (m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123) 
        - m.m03 * (m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123);
    
    det = 1 / det;
    
    mat4 result =
    {
        .m00 = det *   (m.m11 * A2323 - m.m12 * A1323 + m.m13 * A1223),
        .m01 = det * - (m.m01 * A2323 - m.m02 * A1323 + m.m03 * A1223),
        .m02 = det *   (m.m01 * A2313 - m.m02 * A1313 + m.m03 * A1213),
        .m03 = det * - (m.m01 * A2312 - m.m02 * A1312 + m.m03 * A1212),
        .m10 = det * - (m.m10 * A2323 - m.m12 * A0323 + m.m13 * A0223),
        .m11 = det *   (m.m00 * A2323 - m.m02 * A0323 + m.m03 * A0223),
        .m12 = det * - (m.m00 * A2313 - m.m02 * A0313 + m.m03 * A0213),
        .m13 = det *   (m.m00 * A2312 - m.m02 * A0312 + m.m03 * A0212),
        .m20 = det *   (m.m10 * A1323 - m.m11 * A0323 + m.m13 * A0123),
        .m21 = det * - (m.m00 * A1323 - m.m01 * A0323 + m.m03 * A0123),
        .m22 = det *   (m.m00 * A1313 - m.m01 * A0313 + m.m03 * A0113),
        .m23 = det * - (m.m00 * A1312 - m.m01 * A0312 + m.m03 * A0112),
        .m30 = det * - (m.m10 * A1223 - m.m11 * A0223 + m.m12 * A0123),
        .m31 = det *   (m.m00 * A1223 - m.m01 * A0223 + m.m02 * A0123),
        .m32 = det * - (m.m00 * A1213 - m.m01 * A0213 + m.m02 * A0113),
        .m33 = det *   (m.m00 * A1212 - m.m01 * A0212 + m.m02 * A0112),
    };
    
    return result;
}

internal mat4
mat4_lookat(v3 camera_p, v3 target_p)
{
    v3 up = V3(0, 1, 0);
    v3 zAxis = v3_norm(v3_sub(target_p, camera_p));
    v3 xAxis = v3_norm(v3_cross(zAxis, up));
    v3 yAxis = v3_cross(xAxis, zAxis);
    
    mat4 result = mat4_camera_3x3(xAxis, yAxis, zAxis, camera_p);
    result = mat4_invert(result);
    
    return result;
}

internal mat4
mat4_get_translate(void)
{
    mat4 result = 
    {
        
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

internal mat4
mat4_get_scale(void)
{
    mat4 result = 
    {
        
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

internal mat4
mat4_get_rotate_x(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    
    mat4 result = 
    {
        
        1, 0, 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

internal mat4
mat4_get_rotate_y(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    
    mat4 result = 
    {
        
        c, 0, s, 0,
        0, 1, 0, 0,
        -s, 0, c, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

internal mat4
mat4_get_rotate_z(f32 angle)
{
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    
    mat4 result = 
    {
        
        c, -s, 0, 0,
        s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

#endif //MATH_MAT4_H
