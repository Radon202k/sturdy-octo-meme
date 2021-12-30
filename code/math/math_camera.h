/* date = December 29th 2021 11:30 pm */

#ifndef MATH_CAMERA_H
#define MATH_CAMERA_H

internal mat4_inv
mat4_perspective(f32 ar, f32 fov, f32 zNear, f32 zFar)
{
    f32 a = 1.0f;
    f32 b = ar;
    f32 c = fov; // NOTE(casey): This should really be called "film back distance"
    
    f32 n = zNear; // NOTE(casey): Near clip plane _distance_
    f32 f = zFar; // NOTE(casey): Far clip plane _distance_
    
    // NOTE(casey): These are the perspective correct terms, for when you divide by -z
    f32 d = (n+f) / (n-f);
    f32 e = (2*f*n) / (n-f);
    
    mat4_inv result =
    {
        // NOTE(casey): Forward
        {
            a*c,  0,  0,  0,
            0,  b*c,  0,  0,
            0,    0,  d,  e,
            0,    0, -1,  0},
        
        // NOTE(casey): Inverse
        {
            1/(a*c), 0,   0,   0,
            0, 1/(b*c),   0,   0,
            0,       0,   0,  -1,
            0,       0, 1/e, d/e
        },
    };
    
    return(result);
}

internal mat4_inv
camera_transform(v3 X, v3 Y, v3 Z, v3 P)
{
    mat4_inv result;
    
    mat4 A = mat4_rows_3x3(X, Y, Z);
    v3 AP = v3_neg(mat4_mul_v3(A, P));
    A = mat4_translate(A, AP);
    result.forward = A;
    
    v3 iX = v3_div(X, v3_lengthSq(X));
    v3 iY = v3_div(Y, v3_lengthSq(Y));
    v3 iZ = v3_div(Z, v3_lengthSq(Z));
    v3 iP = {AP.x*iX.x + AP.y*iY.x + AP.z*iZ.x,
        AP.x*iX.y + AP.y*iY.y + AP.z*iZ.y,
        AP.x*iX.z + AP.y*iY.z + AP.z*iZ.z};
    
    mat4 B = mat4_cols_3x3(iX, iY, iZ);
    B = mat4_translate(B, v3_neg(iP));
    result.inverse = B;
    
    return(result);
}

internal mat4
build_camera_object_matrix(v3 offset, f32 orbit, f32 pitch, f32 dolly)
{
    mat4 result = mat4_mul(mat4_translation(offset),mat4_mul(mat4_zrotation(orbit),mat4_mul(mat4_xrotation(pitch),mat4_translation(V3(0, 0, dolly)))));
    return(result);
}

#endif //MATH_CAMERA_H
