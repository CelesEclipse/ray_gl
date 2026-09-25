#pragma once

#include "raylib.h"

void camera_update(Camera3D * camera, float * camera_angleH, float * camera_angleV, Vector3 player_pos);
void camera_get_basis(Camera3D camera, Vector3 * forward, Vector3 * right);
