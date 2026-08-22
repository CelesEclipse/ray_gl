#pragma once

#include <raylib.h>

typedef struct CollisionResult_t {
    Vector3 c_movement;
    Vector3 c_correction;
} CollisionResult_t;

CollisionResult_t collision_resolve_aabb(BoundingBox b1, BoundingBox b2, Vector3 movement);
Vector3 collision_get_pushout(BoundingBox b1, BoundingBox b2);
