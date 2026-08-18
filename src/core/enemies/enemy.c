#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "enemy.h"

#define NAME_SIZE   50
#define MAX_HP      100

typedef enum
{
    IDLE,
    MOVING,
    ATTACK,
    DEAD
} EnemyState_t;

struct Enemy
{
    char            m_name[NAME_SIZE];
    Vector3         m_pos;
    float           m_hp;
    float           m_speed;
    float           m_atk_range;
    float           m_atk_dmg;
    float           m_rotation;
    EnemyState_t    m_state;
};

Enemy_t * enemy_initialize(const char * name)
{
    Enemy_t * e = (Enemy_t *)malloc(sizeof(Enemy_t));
    if (e == NULL) return NULL;
    
    strncpy(e->m_name, name, sizeof(e->m_name) - 1);
    e->m_name[sizeof(e->m_name) - 1] = '\0';

    /* Other features */
    e->m_pos = (Vector3){0.0f, 1.0f, -5.0f};
    e->m_speed = 5.0f;
    e->m_hp = 100.0f;
    e->m_atk_range = 1.5f;
    e->m_atk_dmg = 20.0f;
    e->m_rotation = 0.0f;
    e->m_state = IDLE;

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
    return enemy->m_pos;
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

void enemy_update_general(Enemy_t * enemy, Vector3 * out_pos, Vector3 * player_pos, float deltatime)
{
    if (enemy == NULL || out_pos == NULL || player_pos == NULL) return;
    if (enemy->m_state == DEAD) return;

    Vector3 dist = Vector3Subtract(*player_pos, *out_pos);
    float distance = Vector3Length(dist);
    printf("Dist: %.2f | AtkRange: %.2f | State: %d\n", distance, enemy->m_atk_range, enemy->m_state);

    if (distance > enemy->m_atk_range) {
        enemy->m_state = MOVING;
        Vector3 norm_vec = Vector3Normalize(dist);
        Vector3 scaled_vec = Vector3Scale(norm_vec, (enemy->m_speed * deltatime));
        *out_pos = Vector3Add(*out_pos, scaled_vec);

        // Update back
        enemy->m_pos = *out_pos;
    }

    if (distance <= enemy->m_atk_range) {
        enemy->m_state = ATTACK;
    }
}
