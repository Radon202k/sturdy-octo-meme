/* date = December 29th 2021 11:30 pm */

#ifndef MATH_CAMERA_H
#define MATH_CAMERA_H

typedef struct camera_t
{
    v3 pos;
    f32 pitch;
    f32 yaw;
    f32 roll;
    
    transform_t *targetTransform;
    
    f32 distanceFromTarget;
    f32 angleAroundTarget;
} camera_t;

internal mat4
camera_make_view_matrix(camera_t *camera)
{
    mat4 result = mat4_xrotation(camera->pitch);
    result = mat4_mul(result, mat4_yrotation(camera->yaw));
    result = mat4_mul(result, mat4_translation(v3_neg(camera->pos)));
    return result;
}

internal void
camera_update(camera_t *camera)
{
    f32 horizontalDistance = cosf(camera->pitch) * camera->distanceFromTarget;
    f32 verticalDistance = sinf(camera->pitch) * camera->distanceFromTarget;
    
    v3 targetRot = camera->targetTransform->rotation;
    v3 targetPos = camera->targetTransform->translation;
    
    f32 theta = targetRot.y + camera->angleAroundTarget;
    
    f32 offsetX = horizontalDistance * sinf(theta);
    f32 offsetZ = horizontalDistance * cosf(theta);
    
    // Update camera's position
    v3 camPos = 
    {
        .x = targetPos.x - offsetX,
        .y = targetPos.y + verticalDistance,
        .z = targetPos.z - offsetZ,
    };
    camera->pos = camPos;
    
    // yaw
    camera->yaw = 3.1415f - (targetRot.y + camera->angleAroundTarget);
}

#endif //MATH_CAMERA_H
