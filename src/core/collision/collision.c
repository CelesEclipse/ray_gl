#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <raymath.h>
#include "collision.h"

/* Simple AABB implementation */
CollisionResult_t collision_resolve_aabb(BoundingBox b1, BoundingBox b2, Vector3 movement)
{
    CollisionResult_t res = {
        .c_movement = movement,
        .c_correction = (Vector3){0}
    };

    // ahh, I just move from main ..
    if (CheckCollisionBoxes(b1, b2)) {
        float overlap_x = fminf(b1.max.x, b2.max.x) - fmaxf(b1.min.x, b2.min.x);
        float overlap_y = fminf(b1.max.y, b2.max.y) - fmaxf(b1.min.y, b2.min.y);
        float overlap_z = fminf(b1.max.z, b2.max.z) - fmaxf(b1.min.z, b2.min.z);
        Vector3 normal = {0.0f};

        if (overlap_x < overlap_y && overlap_x < overlap_z) {
            normal.x = (b1.min.x < b2.min.x) ? -1.0f : 1.0f;
        }
        else if (overlap_y < overlap_z) {
            normal.y = (b1.min.y < b2.min.y) ? -1.0f : 1.0f;
        }
        else {
            normal.z = (b1.min.z < b2.min.z) ? -1.0f : 1.0f;
        }

        // Calculate slide vector
        float into_surface = Vector3DotProduct(res.c_movement, normal);
        if (into_surface < 0.0f) {
            res.c_movement = Vector3Subtract(res.c_movement, Vector3Scale(normal, into_surface));
        }
        res.c_correction = collision_get_pushout(b1, b2);
    }
    return res;
}

Vector3 collision_get_pushout(BoundingBox b1, BoundingBox b2)
{
    float overlap_x = fminf(b1.max.x, b2.max.x) - fmaxf(b1.min.x, b2.min.x);
    float overlap_y = fminf(b1.max.y, b2.max.y) - fmaxf(b1.min.y, b2.min.y);
    float overlap_z = fminf(b1.max.z, b2.max.z) - fmaxf(b1.min.z, b2.min.z);
    Vector3 pushout = {0};

    if (overlap_x < overlap_y && overlap_x < overlap_z) {
        pushout.x = (b1.min.x < b2.min.x) ? -overlap_x : overlap_x;
    } else if (overlap_y < overlap_z) {
        pushout.y = (b1.min.y < b2.min.y) ? -overlap_y : overlap_y;
    } else {
        pushout.z = (b1.min.z < b2.min.z) ? -overlap_z : overlap_z;
    }

    return pushout;
}

bool collision_check_hitbox(BoundingBox atk_hitbox, BoundingBox def_hurtbox)
{
    return (atk_hitbox.min.x <= def_hurtbox.max.x && atk_hitbox.max.x >= def_hurtbox.min.x) &&
            (atk_hitbox.min.y <= def_hurtbox.max.y && atk_hitbox.max.y >= def_hurtbox.min.y) &&
            (atk_hitbox.min.z <= def_hurtbox.max.z && atk_hitbox.max.z >= def_hurtbox.min.z);
}

/*
A simple collision between capsule vs sphere
- Just need to care about the intersection point, no need to care about the shape
*/
CollisionResult_Capsule_t collision_resolve_capsule_sphere(CapsuleCollider3D_t * capsule, SphereCollider_t * sphere)
{
    if (!capsule || !sphere) goto errout;

    CollisionResult_Capsule_t ret;

    // 1. Calculate the projection vector of sphere's center to (base - tip)
    Vector3 capsule_base = geometry_capsule_get_coord(capsule, 0);
    Vector3 capsule_tip = geometry_capsule_get_coord(capsule, 1);
    Vector3 sphere_center = geometry_sphere_get_center(sphere);

    Vector3 sphere_center_to_capsule = geometry_point_to_segment_projection(sphere_center, capsule_base, capsule_tip);

    // 2. Create a distance vector from the projection to the sphere center
    Vector3 delta_vec = Vector3Subtract(sphere_center, sphere_center_to_capsule);
    float target_dist = geometry_capsule_get_radius(capsule) + geometry_sphere_get_radius(sphere);
    float actual_dist = Vector3Length(delta_vec);

    if (FloatEquals(actual_dist, 0.0f)) {
        ret.normal_vector = (Vector3){0.0f, 1.0f, 0.0f};
        ret.penetration_depth = target_dist;
    }

    // 3. Calculate penetration depth
    float pd = target_dist - actual_dist;
    if (pd <= 0) {
        goto errout;
    } else {
        ret.normal_vector = Vector3Normalize(delta_vec);
        ret.penetration_depth = pd;
    }

    return ret;

    errout:
        CollisionResult_Capsule_t null_res = {
            .normal_vector = (Vector3){0.0f, 0.0f, 0.0f},
            .penetration_depth = 0.0f
        };
        return null_res;
}

/*
Collision between capsule vs sphere
*/
CollisionResult_Capsule_t collision_resolve_capsule_box(CapsuleCollider3D_t * capsule, BoundingBox box)
{
    if (!capsule) goto errout;

    CollisionResult_Capsule_t ret;

    // 1. I think umm.. it may not just simply replace the sphere center to the box center
    // but the projection kinda the same
    Vector3 capsule_base = geometry_capsule_get_coord(capsule, 0);
    Vector3 capsule_tip = geometry_capsule_get_coord(capsule, 1);
    Vector3 box_center = Vector3Zero();
    box_center.x = (box.min.x + box.max.x) * 0.5f;
    box_center.y = (box.min.y + box.max.y) * 0.5f;
    box_center.z = (box.min.z + box.max.z) * 0.5f;

    Vector3 capsule_point = geometry_point_to_segment_projection(box_center, capsule_base, capsule_tip);

    // 2. Clamp capsule point into box (the closest point inside/surface of the box)
    Vector3 box_point = Vector3Zero();
    box_point.x = fmaxf(box.min.x, fminf(box.max.x, capsule_point.x));
    box_point.y = fmaxf(box.min.y, fminf(box.max.y, capsule_point.y));
    box_point.z = fmaxf(box.min.z, fminf(box.max.z, capsule_point.z));

    // 3. convert it to the spherical case huh
    capsule_point = geometry_point_to_segment_projection(box_point, capsule_base, capsule_tip);
    Vector3 delta_vec = Vector3Subtract(capsule_point, box_point);
    float actual_dist = Vector3Length(delta_vec);
    float target_dist = geometry_capsule_get_radius(capsule);
    float pd = target_dist - actual_dist;

    if (pd <= 0) {
        goto errout;
    } else {
        ret.normal_vector = Vector3Normalize(delta_vec);
        ret.penetration_depth = pd;
    }
    return ret;

    errout:
        CollisionResult_Capsule_t null_res = {
            .normal_vector = (Vector3){0.0f, 0.0f, 0.0f},
            .penetration_depth = 0.0f
        };
        return null_res;
}
