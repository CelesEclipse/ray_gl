#pragma once

#include "raylib.h"
#include "raymath.h"

typedef struct Enemy Enemy_t;

Enemy_t * enemy_initialize(const char * name);
void enemy_destroy(Enemy_t * enemy);

/* Getter functions */
Vector3 enemy_get_position(const Enemy_t * enemy);
float   enemy_get_speed(const Enemy_t * enemy);
float   enemy_get_rotation(const Enemy_t * enemy);
void    enemy_update_general(Enemy_t * enemy, Vector3 * out_pos, Vector3 * player_pos, float deltatime);