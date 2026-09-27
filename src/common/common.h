#pragma once

#include "raylib.h"
#define CAPSULE_HEIGHT  2.8f

typedef enum
{
    ANIM_IDLE,
    ANIM_WALK,
    ANIM_RUN,
    ANIM_DEATH,
    ANIM_ATK,
    ANIM_ATK_360
} AnimState_t;

typedef enum
{
    ATK_NONE,
    ATK_QUICK,
    ATK_360
} AttackRequest_t;

typedef enum
{
    ITEM_COIN,
    ITEM_BOMB,
    ITEM_COUNT
} ItemType_t;

typedef struct 
{
    ItemType_t type;
    int quantity;
} Item_t;
