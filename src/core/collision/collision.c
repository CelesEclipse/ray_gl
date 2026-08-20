#include "collision.h"
#include <raymath.h>

/* Simple AABB implementation */
Vector3 collision_resolve_aabb(BoundingBox b1, BoundingBox b2, Vector3 movement)
{
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
        float into_surface = Vector3DotProduct(movement, normal);
        if (into_surface < 0.0f) {
            movement = Vector3Subtract(movement, Vector3Scale(normal, into_surface));
        }
    }
    return movement;
}
