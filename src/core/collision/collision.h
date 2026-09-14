#pragma once

#include <raylib.h>
#include "../../physics/geometry.h"


typedef struct CollisionResult_t 
{
    Vector3 c_movement;
    Vector3 c_correction;
} CollisionResult_t;

typedef struct CollisionResult_Capsule_t
{
    Vector3 normal_vector;
    float penetration_depth;
} CollisionResult_Capsule_t;

CollisionResult_t collision_resolve_aabb(BoundingBox b1, BoundingBox b2, Vector3 movement);
Vector3 collision_get_pushout(BoundingBox b1, BoundingBox b2);
bool collision_check_hitbox(BoundingBox atk_hitbox, BoundingBox def_hurtbox);

/* For capsule - polygons */
CollisionResult_Capsule_t collision_resolve_capsule_sphere(CapsuleCollider3D_t * capsule, SphereCollider_t * sphere);
CollisionResult_Capsule_t collision_resolve_capsule_box(CapsuleCollider3D_t * capsule, BoundingBox box);
CollisionResult_Capsule_t collision_resolve_capsules(CapsuleCollider3D_t * capsule1, CapsuleCollider3D_t * capsule2);

