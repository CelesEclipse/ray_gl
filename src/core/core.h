#pragma once

#include <raylib.h>
#include "player/player.h"
#include "enemies/enemy.h"

#define     ATK_IMPACT_FRAME            12
#define     ATK360_IMPACT_FRAME         24

#define     ATK_DAMAGE          10.0f
#define     ATK360_DAMAGE       20.0f
#define     ENEMY_ATK_DAMAGE    10.0f

/*
Assume each enemy's collider has been updated for this frame
This function only snapshots and updates the PLAYER's own collider
internally, since that's purely a combat-local concern
*/

void combat_resolve(
    Player_t * pl,
    Enemy_t ** enemies,
    int count,
    Vector3 pl_pos,
    int pl_anim_frame,
    Vector3 * out_pl_correction
);
