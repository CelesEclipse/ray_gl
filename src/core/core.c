#include "core.h"
#include "collision/collision.h"

#include <raymath.h>

void combat_resolve(
    Player_t * pl,
    Enemy_t ** enemies,
    int count,
    Vector3 pl_pos,
    int pl_anim_frame,
    Vector3 * out_pl_correction
)

{
    if (!pl || !enemies || !out_pl_correction) return;

    *out_pl_correction = (Vector3){0};

    player_update_collider(pl);
    CapsuleCollider3D_t * player_collider_snapshot = player_get_collider(pl);

    // Physical collision (not hitbox)
    for (int i = 0; i < count; ++i) {
        CollisionResult_Capsule_t body_col = collision_resolve_capsules(
            player_collider_snapshot,
            enemy_get_collider(enemies[i])
        );
        if (body_col.penetration_depth > 0.0f) {
            Vector3 push = Vector3Scale(body_col.normal_vector, body_col.penetration_depth);
            *out_pl_correction = Vector3Add(*out_pl_correction, push);
        }
    }

    // hitbox, damage checks
    for (int i = 0; i < count; ++i) {
        if (!enemies[i]) continue;

        if (enemy_get_anim_current(enemies[i]) == ANIM_ATK && 
            enemy_get_anim_frame(enemies[i]) == ATK_IMPACT_FRAME) {
            CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                player_collider_snapshot,
                enemy_get_hitbox(enemies[i], pl_pos)
            );
            if (hit.penetration_depth > 0.0f) {
                player_take_damage(pl, ENEMY_ATK_DAMAGE);
            }
        }

        // normal attack
        if (player_get_anim_current(pl) == ANIM_ATK && 
            pl_anim_frame == ATK_IMPACT_FRAME) {
            CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                enemy_get_collider(enemies[i]), 
                player_get_hitbox(pl)
            );
            if (hit.penetration_depth > 0.0f) {
                enemy_take_damage(enemies[i], ATK_DAMAGE);
            }
        }

        // strong attack , ah it's 360 
        if (player_get_anim_current(pl) == ANIM_ATK_360 &&
         pl_anim_frame == ATK360_IMPACT_FRAME) {
            CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                enemy_get_collider(enemies[i]), 
                player_get_hitbox(pl)
            );
            if (hit.penetration_depth > 0.0f) {
                enemy_take_damage(enemies[i], ATK360_DAMAGE);
            }
        }
    }
}
