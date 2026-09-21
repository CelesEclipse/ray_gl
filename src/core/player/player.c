#include <string.h>
#include <stdlib.h>
#include "player.h"
#include "raylib.h"
#include "raymath.h"
#include "../../common/common.h"

#define NAME_SIZE       50
#define MAX_HP          100

struct Player
{
    char            m_name[NAME_SIZE];
    Vector3         m_position;
    Vector3         m_direction;
    float           m_hp;
    float           m_speed;
    float           m_atk_range;
    float           m_atk_speed;
    float           m_atk_timer;
    float           m_atk_dmg;
    float           m_rotation;
    bool            m_did_atk_this_tick;
    PlayerState_t   m_state;
    CapsuleCollider3D_t * m_collider;
    int             m_anim_current;
    int             m_anim_frame;
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
    p->m_hp = 94.0f;
    p->m_atk_range = 1.5f;
    p->m_atk_speed = 2.0f;
    p->m_atk_timer = 0.0f;
    p->m_atk_dmg = 20.0f;
    p->m_rotation = 0.0f;
    p->m_did_atk_this_tick = false;
    p->m_state = P_IDLE;
    p->m_anim_current = ANIM_IDLE;
    p->m_anim_frame = 0;

    p->m_collider = geometry_capsule_alloc();
    geometry_capsule_set_radius(p->m_collider, 1.0f);
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

float player_get_hp(const Player_t * player)
{
    if (player == NULL) return 0.0f;
    return player->m_hp;
}

int player_get_state(const Player_t * player)
{
    if (player == NULL) return 0;
    return (int)player->m_state;
}

bool player_is_dead(const Player_t * player)
{
    if (player == NULL) return false;
    return player->m_state == P_DEAD;
}

bool player_get_did_attack(const Player_t * player)
{
    if (player == NULL) return false;
    return player->m_did_atk_this_tick;
}

CapsuleCollider3D_t * player_get_collider(const Player_t * player)
{
    if (player == NULL) return NULL;
    return player->m_collider;
}

BoundingBox player_get_hitbox(const Player_t * player)
{
    if (player == NULL) return (BoundingBox){0};

    Vector3 facing = {sinf(player->m_rotation * DEG2RAD), 0.0f, cosf(player->m_rotation * DEG2RAD)};
    Vector3 hitbox_centre = Vector3Add(player->m_position, Vector3Scale(facing, player->m_atk_range * 0.5f));

    float half_size = player->m_atk_range * 0.5;
    return (BoundingBox){
        .min = (Vector3){hitbox_centre.x - half_size, player->m_position.y, hitbox_centre.z - half_size},
        .max = (Vector3){hitbox_centre.x + half_size, player->m_position.y + 2.0f, hitbox_centre.z + half_size}
    };
}

int player_get_anim_current(const Player_t * player)
{
    if (!player) return -1;
    return player->m_anim_current;
}

int player_get_anim_frame(const Player_t * player)
{
    if (!player) return -1;
    return player->m_anim_frame;
}

void player_set_position(Player_t * player, Vector3 new_pos)
{
    if (player == NULL) return;

    player->m_position = new_pos;
}

void player_set_sprint(Player_t * player, bool sprint)
{
    // hmm, just hard-coded for now
    if (!player) return;
    if (sprint) {
        player->m_speed = 18.0f;
    } else {
        player->m_speed = 8.0f;
    }
}

void player_set_hp(Player_t * player, float hp)
{
    if (player == NULL) return;
    player->m_hp += hp;
    if (player->m_hp < 0) player->m_hp = 0;
}

void player_normal_attack(Player_t * player, float deltatime)
{
    if (player == NULL) return;
    if (player->m_state == P_DEAD) return;

    player->m_atk_timer += deltatime;

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float atk_interval = 1.0f / player->m_atk_speed;
        if (player->m_atk_timer >= atk_interval) {
            player->m_atk_timer = 0.0f;
            player->m_state = P_ATTACK;
            player->m_did_atk_this_tick = true;
        } else {
            player->m_did_atk_this_tick = false;
        }
    } else {
        player->m_did_atk_this_tick = false;
    }
}

void player_take_damage(Player_t * player, float recv_dmg)
{
    if (player == NULL) return;
    player->m_hp -= recv_dmg;
    if (player->m_hp < 0) player->m_hp = 0;
    if (player->m_hp <= 0) player->m_state = P_DEAD;
}

void player_update_collider(Player_t * player)
{
    if (player == NULL) return;

    float rad = geometry_capsule_get_radius(player->m_collider);
    float segment_len = CAPSULE_HEIGHT - (2.0f * rad);
    if (segment_len < 0.0f) segment_len = 0.0f;

    Vector3 base = {
        player->m_position.x,
        player->m_position.y + rad,
        player->m_position.z
    };
    Vector3 tip = {
        base.x, base.y + segment_len, base.z
    };
    geometry_capsule_set_coord(player->m_collider, 0, base);
    geometry_capsule_set_coord(player->m_collider, 1, tip);
}

void player_set_anim(Player_t * player, int anim_idx)
{
    if (!player) return;
    if (player->m_anim_current == anim_idx) return;

    player->m_anim_current = anim_idx;
    player->m_anim_frame = 0;
}

void player_set_anim_frame(Player_t * player, int frame)
{
    if (!player) return;
    player->m_anim_frame = frame;
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
    if (player == NULL || out_rotation == NULL) return (Vector3){0};
    if (player->m_state == P_DEAD)    return (Vector3){0};

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
        player->m_state = P_MOVING;

        return Vector3Scale(
            moveDirection,
            player->m_speed * deltatime
        );
    }

    player->m_state = P_IDLE;
    return (Vector3){0};
}
