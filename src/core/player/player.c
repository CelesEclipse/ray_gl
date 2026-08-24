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
    Vector3         m_position;
    Vector3         m_direction;
    float           m_hp;
    float           m_speed;
    float           m_atk_range;
    float           m_atk_dmg;
    float           m_rotation;
    PlayerState_t   m_state;
    BoundingBox     m_collider;
};

Player_t * player_initialize(const char * name)
{
    Player_t * p = (Player_t *)malloc(sizeof(Player_t));
    if (p == NULL) return NULL;
    
    strncpy(p->m_name, name, sizeof(p->m_name) - 1);
    p->m_name[sizeof(p->m_name) - 1] = '\0';

    /* Other features */
    p->m_position = (Vector3){0.0f, 0.0f, 0.0f};
    p->m_direction = (Vector3){0.0f, 0.0f, 0.0f};
    p->m_speed = 8.0f;
    p->m_hp = 100.0f;
    p->m_atk_range = 1.5f;
    p->m_atk_dmg = 20.0f;
    p->m_rotation = 0.0f;
    p->m_state = IDLE;
    p->m_collider.min = (Vector3){-0.5f, 0.0f, -0.5f};
    p->m_collider.max = (Vector3){0.5f, 2.0f, 0.5f};
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
    return player->m_position;
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

BoundingBox player_get_collider(const Player_t * player)
{
    if (player == NULL) return (BoundingBox){0};
    return player->m_collider;
}

void player_set_position(Player_t * player, Vector3 new_pos)
{
    if (player == NULL) return;

    player->m_position = new_pos;
}

void player_update_collider(Player_t * player)
{
    if (player == NULL) return;

    player->m_collider.min = (Vector3){
        player->m_position.x - 0.5f,
        player->m_position.y,
        player->m_position.z - 0.5f
    };

    player->m_collider.max = (Vector3){
        player->m_position.x + 0.5f,
        player->m_position.y + 2.0f,
        player->m_position.z + 0.5f
    };
}

Vector3 player_update_general(
    Player_t *player,
    float *out_rotation,
    float deltatime,
    Vector3 forward,
    Vector3 right
)
{
    /* Refactor, return movement instead of synch between struct and main */
    if (player == NULL || out_rotation == NULL)
        return (Vector3){0};

    Vector3 moveDirection = {0};

    if (IsKeyDown(KEY_W))
        moveDirection = Vector3Add(moveDirection, forward);
    if (IsKeyDown(KEY_S))
        moveDirection = Vector3Subtract(moveDirection, forward);
    if (IsKeyDown(KEY_A))
        moveDirection = Vector3Subtract(moveDirection, right);
    if (IsKeyDown(KEY_D))
        moveDirection = Vector3Add(moveDirection, right);

    if (Vector3Length(moveDirection) > 0.0f) {
        moveDirection = Vector3Normalize(moveDirection);
        *out_rotation = atan2f(moveDirection.x, moveDirection.z) * RAD2DEG;

        player->m_rotation = *out_rotation;

        return Vector3Scale(
            moveDirection,
            player->m_speed * deltatime
        );
    }

    return (Vector3){0};
}

static void player_attack(void)
{

}

static void player_take_damage(void)
{

}

static bool player_is_attacking(void)
{
    return true;
}