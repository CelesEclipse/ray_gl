#include "camera.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>

void camera_update(Camera3D * camera, float * camera_angleH, float * camera_angleV, Vector3 player_pos)
{
    if (!camera && !camera_angleH && !camera_angleV) return;

    camera->position = (Vector3){0.0f, 5.0f, 6.0f};
    camera->target = player_pos;
    camera->up = (Vector3){0.0f, 1.0f, 0.0f};
    camera->fovy = 60.0f;
    camera->projection = CAMERA_PERSPECTIVE;

    Vector2 mouseDelta = GetMouseDelta();
    *camera_angleH -= mouseDelta.x * 0.003f;
    *camera_angleV += mouseDelta.y * 0.003f;

    if (*camera_angleV > 1.2f) *camera_angleV = 1.2f;
    if (*camera_angleV < 0.1f) *camera_angleV = 0.1f;

    float cameraRadius = 8.0f;
    camera->position.x = player_pos.x + cameraRadius * sinf(*camera_angleH) * cosf(*camera_angleV);
    camera->position.z = player_pos.z + cameraRadius * cosf(*camera_angleH) * cosf(*camera_angleV);
    camera->position.y = player_pos.y + cameraRadius * sinf(*camera_angleV);
    camera->target = player_pos;
}

void camera_get_basis(Camera3D camera, Vector3 * forward, Vector3 * right)
{
    if (!forward && !right) return;

    forward->y = 0.0f;
    *forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
    *forward = Vector3Normalize(*forward);
    
    *right = (Vector3){
        -forward->z,
        0.0f,
        forward->x
    };
}