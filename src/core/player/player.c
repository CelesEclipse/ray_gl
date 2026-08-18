#include <string.h>
#include <stdlib.h>
#include "player.h"

#define NAME_SIZE   50
#define MAX_HP      100

typedef enum
{
    IDLE,
    MOVING,
    ATTACK,
    DEAD
} PlayerState_t;

struct Player
{
    char            m_name[NAME_SIZE];
    Vector3         m_pos;
    Vector3         m_direction;
    float           m_hp;
    float           m_speed;
    float           m_atk_range;
    float           m_atk_dmg;
    float           m_rotation;
    PlayerState_t   m_state;
};

Player_t * player_initialize(const char * name)
{
    Player_t * p = (Player_t *)malloc(sizeof(Player_t));
    if (p == NULL) return NULL;
    
    strncpy(p->m_name, name, sizeof(p->m_name) - 1);
    p->m_name[sizeof(p->m_name) - 1] = '\0';

    /* Other features */
    p->m_pos = (Vector3){0.0f, 0.0f, 0.0f};
    p->m_direction = (Vector3){0.0f, 0.0f, 0.0f};
    p->m_speed = 8.0f;
    p->m_hp = 100.0f;
    p->m_atk_range = 1.5f;
    p->m_atk_dmg = 20.0f;
    p->m_rotation = 0.0f;
    p->m_state = IDLE;

    return p;
}

void player_destroy(Player_t * pl)
{
    if (pl != NULL) {
        free(pl);
    }
    pl = NULL;
}

Vector3 player_get_position(const Player_t * player)
{
    if (player == NULL) return Vector3Zero();
    return player->m_pos;
}

Vector3 player_get_direction(const Player_t * player)
{
    if (player == NULL) return Vector3Zero();
    return player->m_direction;
}

float player_get_speed(const Player_t * player)
{
    if (player == NULL) return 0.0f;
    return player->m_speed;
}

float player_get_rotation(const Player_t * player)
{
    if (player == NULL) return 0.0f;
    return player->m_rotation;
}

void player_update_general(Player_t * player, Vector3 * out_pos, float * out_rotation, float deltatime, Vector3 forward, Vector3 right)
{
    if (player == NULL || out_pos == NULL || out_rotation == NULL) return;

    Vector3 moveDirection = (Vector3){0.0f, 0.0f, 0.0f};
    if (IsKeyDown(KEY_W)) moveDirection = Vector3Add(moveDirection, forward);
    if (IsKeyDown(KEY_S)) moveDirection = Vector3Subtract(moveDirection, forward);
    if (IsKeyDown(KEY_A)) moveDirection = Vector3Subtract(moveDirection, right);
    if (IsKeyDown(KEY_D)) moveDirection = Vector3Add(moveDirection, right);

    /* Need to synchronize to the variable in main */
    if (Vector3Length(moveDirection) > 0.0f) {
        moveDirection = Vector3Normalize(moveDirection);
        *out_pos = Vector3Add(*out_pos, Vector3Scale(moveDirection, player->m_speed * deltatime));
        *out_rotation = atan2f(moveDirection.x, moveDirection.z) * RAD2DEG;

        // Update back to struct
        player->m_pos = *out_pos;
        player->m_rotation = *out_rotation;
    }
}

static void player_attack(void)
{

}

static void player_take_damage(void)
{

}

static bool player_is_attacking(void)
{

}