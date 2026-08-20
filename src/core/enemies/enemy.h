#pragma once

#include "raylib.h"
#include "raymath.h"

typedef struct Enemy Enemy_t;

Enemy_t * enemy_initialize(const char * name);
void enemy_destroy(Enemy_t * enemy);

/* Getter functions */
Vector3     enemy_get_position(const Enemy_t * enemy);
float       enemy_get_speed(const Enemy_t * enemy);
float       enemy_get_rotation(const Enemy_t * enemy);
BoundingBox enemy_get_collider(const Enemy_t * enemy);
void        enemy_set_position(Enemy_t * player, Vector3 new_pos);
void        enemy_update_collider(Enemy_t * enemy);
Vector3     enemy_update_general(Enemy_t * enemy, Vector3 player_pos, float deltatime);
