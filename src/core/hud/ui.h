#pragma once

#include <raylib.h>
#include "../enemies/enemy.h"

#define ENEMY_HPBAR_WIDTH   100

void display_hp_bar(int x, int y, int width, int height, float hp, float max_hp);
void set_enemies_hpbar_position(
    Enemy_t ** enemy_list,
    Vector3 * enemy_positions,
    Camera3D camera,
    int count,
    int * bar_coord_x,
    int * bar_coord_y
);
