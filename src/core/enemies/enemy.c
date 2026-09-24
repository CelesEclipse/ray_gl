#include <string.h>
#include <stdlib.h>
#include "enemy.h"
#include "raylib.h"
#include "raymath.h"
#include "../../common/common.h"

#define NAME_SIZE   50
#define MAX_HP      100

struct Enemy
{
    char            m_name[NAME_SIZE];
    Vector3         m_position;
    float           m_hp;
    float           m_speed;
    float           m_atk_speed;
    float           m_atk_timer;
    float           m_atk_range;
    float           m_atk_dmg;
    float           m_detect_range;
    float           m_rotation;
    bool            m_did_atk_this_tick;
    EnemyState_t    m_state;
    CapsuleCollider3D_t * m_collider;
    int             m_anim_current;
    int             m_anim_frame;
};

Enemy_t * enemy_initialize(const char * name)
{
    Enemy_t * e = (Enemy_t *)malloc(sizeof(Enemy_t));
    if (e == NULL) return NULL;
    
    strncpy(e->m_name, name, sizeof(e->m_name) - 1);
    e->m_name[sizeof(e->m_name) - 1] = '\0';

    /* Other features */
    e->m_position = (Vector3){0.0f, 1.0f, -5.0f};
    e->m_speed = 6.0f;
    e->m_hp = 88.2f;
    e->m_atk_speed = 1.0f;
    e->m_atk_timer = 0.0f;
    e->m_atk_range = 1.5f;
    e->m_atk_dmg = 20.0f;
    e->m_detect_range = 10.0f;
    e->m_rotation = 0.0f;
    e->m_did_atk_this_tick = false;
    e->m_state = E_IDLE;
    e->m_anim_current = ANIM_IDLE;
    e->m_anim_frame = 0;

    e->m_collider = geometry_capsule_alloc();
    geometry_capsule_set_radius(e->m_collider, 1.0f);
    return e;
}

void enemy_destroy(Enemy_t * e)
{
    if (e != NULL) {
        free(e);
    }
    e = NULL;
}

Vector3 enemy_get_position(const Enemy_t * enemy)
{
    if (enemy == NULL) return Vector3Zero();
    return enemy->m_position;
}

float enemy_get_speed(const Enemy_t * enemy)
{
    if (enemy == NULL) return 0.0f;
    return enemy->m_speed;
}

float enemy_get_rotation(const Enemy_t * enemy)
{
    if (enemy == NULL) return 0.0f;
    return enemy->m_rotation;
}

float enemy_get_hp(const Enemy_t * enemy)
{
    if (enemy == NULL) return 0.0f;
    return enemy->m_hp;
}

float enemy_get_atk_timer(const Enemy_t * enemy)
{
    if (enemy == NULL) return 0.0f;
    return enemy->m_atk_timer;
}

int enemy_get_state(const Enemy_t * enemy)
{
    if (enemy == NULL) return 0;
    return (int)enemy->m_state;
}

bool enemy_is_dead(const Enemy_t * enemy)
{
    if (enemy == NULL) return false;
    return enemy->m_state == E_DEAD;
}

bool enemy_get_did_attack(const Enemy_t * enemy)
{
    if (enemy == NULL) return false;
    return enemy->m_did_atk_this_tick;
}

BoundingBox enemy_get_hitbox(const Enemy_t * enemy, Vector3 player_pos)
{
    if (enemy == NULL) return (BoundingBox){0};
    Vector3 facing = Vector3Normalize(Vector3Subtract(player_pos, enemy->m_position));
    float reach = enemy->m_atk_range + 1.5f;    // match whatever stop distance actually is
    Vector3 hitbox_centre = Vector3Add(enemy->m_position, Vector3Scale(facing, reach * 0.5f));

    float half_size = reach * 0.5f;
    return (BoundingBox){
        .min = (Vector3){hitbox_centre.x - half_size, enemy->m_position.y, hitbox_centre.z - half_size},
        .max = (Vector3){hitbox_centre.x + half_size, enemy->m_position.y + 2.0f, hitbox_centre.z + half_size}
    };
}

int enemy_get_anim_current(const Enemy_t * enemy)
{
    if (!enemy) return -1;
    return enemy->m_anim_current;
}

int enemy_get_anim_frame(const Enemy_t * enemy)
{
    if (!enemy) return -1;
    return enemy->m_anim_frame;
}


CapsuleCollider3D_t * enemy_get_collider(const Enemy_t * enemy)
{
    if (enemy == NULL) return NULL;
    return enemy->m_collider;
}

void enemy_set_position(Enemy_t * enemy, Vector3 new_pos)
{
    if (enemy == NULL) return;
    enemy->m_position = new_pos;
}

void enemy_set_sprint(Enemy_t * enemy, bool sprint)
{
    // hmm, just hard-coded for now
    if (!enemy) return;
    if (sprint) {
        enemy->m_speed = 16.0f;
    } else {
        enemy->m_speed = 6.0f;
    }
}

void enemy_set_hp(Enemy_t * enemy, float hp)
{
    if (enemy == NULL) return;
    enemy->m_hp += hp;
    if (enemy->m_hp < 0) enemy->m_hp = 0;
}

void enemy_take_damage(Enemy_t * enemy, float amount)
{
    if (enemy == NULL) return;
    enemy->m_hp -= amount;

    if (enemy->m_hp <= 0) {
        enemy->m_state = E_DEAD;
    }
}

void enemy_update_collider(Enemy_t * enemy)
{
    if (enemy == NULL) return;

    float rad = geometry_capsule_get_radius(enemy->m_collider);
    float segment_len = CAPSULE_HEIGHT - (2.0f * rad);
    if (segment_len < 0.0f) segment_len = 0.0f;

    Vector3 base = {
        enemy->m_position.x,
        enemy->m_position.y + rad,
        enemy->m_position.z
    };
    Vector3 tip = {
        base.x, base.y + segment_len, base.z
    };
    geometry_capsule_set_coord(enemy->m_collider, 0, base);
    geometry_capsule_set_coord(enemy->m_collider, 1, tip);
}

Vector3 enemy_update_general(Enemy_t *enemy, Vector3 player_pos, float deltatime)
{
    /* Same as player */
    if (enemy == NULL) return (Vector3){0};
    if (enemy->m_state == E_DEAD) return (Vector3){0};

    Vector3 dist = Vector3Subtract(player_pos, enemy->m_position);
    float distance = Vector3Length(dist);

    // instead of only checking atk_range, add a stop distance
    // to prevent enemies from advancing near contact range
    float stop_distance = enemy->m_atk_range + 1.0f + 0.5f;
    if (distance > enemy->m_detect_range) {
        enemy->m_state = E_IDLE;
        return (Vector3){0};
    }

    if (distance > stop_distance) {
        enemy->m_state = E_MOVING;
        Vector3 direction = Vector3Normalize(dist);

        return Vector3Scale(
            direction,
            enemy->m_speed * deltatime
        );
    }

    enemy->m_state = E_ATTACK;
    enemy->m_rotation = atan2f(dist.x, dist.z) * RAD2DEG;

    return (Vector3){0};
}

void enemy_normal_attack(Enemy_t * enemy, float deltatime)
{
    if (enemy == NULL) return;
    if (enemy->m_state != E_ATTACK) return;

    enemy->m_atk_timer += deltatime;
    float atk_interval = 1.0f / enemy->m_atk_speed;

    if (enemy->m_atk_timer >= atk_interval) {
        enemy->m_atk_timer = 0.0f;
        enemy->m_did_atk_this_tick = true;
    } else {
        enemy->m_did_atk_this_tick = false;
    }
}

void enemy_set_anim(Enemy_t * enemy, int anim_idx)
{
    if (!enemy) return;
    if (enemy->m_anim_current == anim_idx) return;

    enemy->m_anim_current = anim_idx;
    enemy->m_anim_frame = 0;
}

void enemy_set_anim_frame(Enemy_t * enemy, int frame)
{
    if (!enemy) return;
    enemy->m_anim_frame = frame;
}

void enemy_draw_detect_range(Enemy_t * enemy)
{
    if (enemy == NULL) return;

    Vector3 center = enemy->m_position;
    float radius = enemy->m_detect_range;
    Vector3 rot_axis = {1.0f, 0.0f, 0.0f};
    float rot_angle = 90.0f;

    DrawCircle3D(center, radius, rot_axis, rot_angle, YELLOW);
}
