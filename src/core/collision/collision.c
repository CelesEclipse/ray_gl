#include "collision.h"
#include <raymath.h>

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
