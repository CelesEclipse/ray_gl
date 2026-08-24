#pragma once

#include "raylib.h"
#include "raymath.h"

typedef struct Player Player_t;

Player_t * player_initialize(const char * name);
void player_destroy(Player_t * player);

/* Getter functions */
Vector3     player_get_position(const Player_t * player);
Vector3     player_get_direction(const Player_t * player);
float       player_get_speed(const Player_t * player);
float       player_get_rotation(const Player_t * player);
float       player_get_maxhp(const Player_t * player);
float       player_get_hp(const Player_t * player);
BoundingBox player_get_collider(const Player_t * player);

/* Setter functions */
void        player_set_position(Player_t * player, Vector3 new_pos);
void        player_update_collider(Player_t * player);
Vector3     player_update_general(Player_t * player, float * out_rotation, float deltatime, Vector3 forward, Vector3 right);
