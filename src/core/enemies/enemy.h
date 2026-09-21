#pragma once

#include "raylib.h"
#include "../../physics/geometry.h"

typedef enum
{
    E_IDLE,
    E_MOVING,
    E_ATTACK,
    E_DEAD
} EnemyState_t;

typedef struct Enemy Enemy_t;

Enemy_t * enemy_initialize(const char * name);
void enemy_destroy(Enemy_t * enemy);

/* Getter functions */
Vector3     enemy_get_position(const Enemy_t * enemy);
float       enemy_get_speed(const Enemy_t * enemy);
float       enemy_get_rotation(const Enemy_t * enemy);
float       enemy_get_maxhp(const Enemy_t * enemy);
float       enemy_get_hp(const Enemy_t * enemy);
float       enemy_get_atk_timer(const Enemy_t * enemy);
int         enemy_get_state(const Enemy_t  * enemy);
bool        enemy_is_dead(const Enemy_t * enemy);
bool        enemy_get_did_attack(const Enemy_t * enemy);
CapsuleCollider3D_t * enemy_get_collider(const Enemy_t * enemy);
BoundingBox enemy_get_hitbox(const Enemy_t * enemy, Vector3 player_pos);
int         enemy_get_anim_current(const Enemy_t * enemy);
int         enemy_get_anim_frame(const Enemy_t * enemy);

/* Setter functions */
void        enemy_set_position(Enemy_t * enemy, Vector3 new_pos);
void        enemy_set_sprint(Enemy_t * enemy, bool sprint);

/* Update movement and others */
void        enemy_update_collider(Enemy_t * enemy);
void        enemy_set_hp(Enemy_t * enemy, float hp);
void        enemy_take_damage(Enemy_t * enemy, float amount);
void        enemy_normal_attack(Enemy_t * enemy, float deltatime);
void        enemy_set_anim(Enemy_t * enemy, int anim_idx);
void        enemy_set_anim_frame(Enemy_t * enemy, int frame);
void        enemy_draw_detect_range(Enemy_t * enemy);
Vector3     enemy_update_general(Enemy_t * enemy, Vector3 player_pos, float deltatime);
