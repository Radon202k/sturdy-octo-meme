/* date = December 24th 2021 7:42 am */

#ifndef MATH_MAT4_H
#define MATH_MAT4_H

typedef union mat4
{
    float E[4][4];
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

internal mat4
mat4_mul(mat4 A, mat4 B)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    
    mat4 R = {0};
    
    for(int r = 0; r <= 3; ++r) // NOTE(casey): Rows (of A)
    {
        for(int c = 0; c <= 3; ++c) // NOTE(casey): Column (of B)
        {
            for(int i = 0; i <= 3; ++i) // NOTE(casey): Columns of A, rows of B!
            {
                R.E[r][c] += A.E[r][i]*B.E[i][c];
            }
        }
    }
    
    return(R);
}

internal mat4
mat4_cols_3x3(v3 x, v3 y, v3 z)
{
    mat4 result = 
    {
        x.x, y.x, z.x, 0,
        x.y, y.y, z.y, 0,
        x.z, y.z, z.z, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

internal mat4
mat4_rows_3x3(v3 x, v3 y, v3 z)
{
    mat4 result = 
    {
        x.x, x.y, x.z, 0,
        y.x, y.y, y.z, 0,
        z.x, z.y, z.z, 0,
        0, 0, 0, 1,
    };
    
    return result;
}

inline mat4
mat4_xrotation(f32 Angle)
{
    f32 c = cosf(Angle);
    f32 s = sinf(Angle);
    
    mat4 R =
    {
        {{1, 0, 0, 0},
            {0, c,-s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline mat4
mat4_yrotation(f32 Angle)
{
    f32 c = cosf(Angle);
    f32 s = sinf(Angle);
    
    mat4 R =
    {
        {{ c, 0, s, 0},
            { 0, 1, 0, 0},
            {-s, 0, c, 0},
            { 0, 0, 0, 1}},
    };
    
    return(R);
}

inline v3
mat4_get_column(mat4 A, u32 C)
{
    v3 R = {A.E[0][C], A.E[1][C], A.E[2][C]};
    return(R);
}

inline v3
mat4_get_row(mat4 A, u32 R)
{
    v3 result = {A.E[R][0], A.E[R][1], A.E[R][2]};
    return(result);
}

inline mat4
mat4_zrotation(f32 Angle)
{
    f32 c = cosf(Angle);
    f32 s = sinf(Angle);
    
    mat4 R =
    {
        {{c,-s, 0, 0},
            {s, c, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

internal mat4
mat4_translate(mat4 a, v3 t)
{
    mat4 result = a;
    
    result.E[0][3] += t.x;
    result.E[1][3] += t.y;
    result.E[2][3] += t.z;
    
    return(result);
}

inline mat4
mat4_translation(v3 T)
{
    mat4 R =
    {
        {{1, 0, 0, T.x},
            {0, 1, 0, T.y},
            {0, 0, 1, T.z},
            {0, 0, 0, 1}},
    };
    
    return(R);
}

inline mat4
mat4_transpose(mat4 A)
{
    mat4 R;
    
    for(int j = 0; j <= 3; ++j)
    {
        for(int i = 0; i <= 3; ++i)
        {
            R.E[j][i] = A.E[i][j];
        }
    }
    
    return(R);
}

internal v4
mat4_transform(mat4 A, v4 P)
{
    // NOTE(casey): This is written to be instructive, not optimal!
    
    v4 R;
    
    R.x = P.x*A.E[0][0] + P.y*A.E[0][1] + P.z*A.E[0][2] + P.w*A.E[0][3];
    R.y = P.x*A.E[1][0] + P.y*A.E[1][1] + P.z*A.E[1][2] + P.w*A.E[1][3];
    R.z = P.x*A.E[2][0] + P.y*A.E[2][1] + P.z*A.E[2][2] + P.w*A.E[2][3];
    R.w = P.x*A.E[3][0] + P.y*A.E[3][1] + P.z*A.E[3][2] + P.w*A.E[3][3];
    
    return(R);
}

inline v3
mat4_mul_v3(mat4 A, v3 P)
{
    v3 R = mat4_transform(A, V3ToV4(P, 1.0f)).xyz;
    return(R);
}

inline v4
mat4_mul_v4(mat4 A, v4 P)
{
    v4 R = mat4_transform(A, P);
    return(R);
}

#endif //MATH_MAT4_H
