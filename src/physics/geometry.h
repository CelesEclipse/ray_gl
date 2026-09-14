#pragma once

#include <raylib.h>

typedef enum
{
    CAPSULE,
    SPHERE
} Geometry_Type_t;

typedef struct SphereCollider   SphereCollider_t;
typedef struct CapsuleCollider3D CapsuleCollider3D_t;

CapsuleCollider3D_t * geometry_capsule_alloc();
void geometry_capsule_destroy(CapsuleCollider3D_t * capsule);
float geometry_capsule_calculate_height(const CapsuleCollider3D_t * capsule);

// Capsule getter
Vector3     geometry_capsule_get_coord(const CapsuleCollider3D_t * capsule, int type);
float       geometry_capsule_get_radius(const CapsuleCollider3D_t * capsule);

// Capsule setter
void        geometry_capsule_set_coord(CapsuleCollider3D_t * capsule, int type, Vector3 coord);
void        geometry_capsule_set_radius(CapsuleCollider3D_t * capsule, float rad);

SphereCollider_t * geometry_sphere_alloc();
void geometry_sphere_destroy(SphereCollider_t * sphere);

// Sphere getter
Vector3     geometry_sphere_get_center(const SphereCollider_t * sphere);
float       geometry_sphere_get_radius(const SphereCollider_t * sphere);

// Math functions
Vector3 geometry_point_to_segment_projection(Vector3 point, Vector3 tip1, Vector3 tip2);
Vector3 geometry_closest_distance_seg2seg(Vector3 Abase, Vector3 Atip, Vector3 Bbase, Vector3 Btip);
