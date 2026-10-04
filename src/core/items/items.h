#pragma once

#include "raylib.h"
#include "../../common/common.h"
#include <stdbool.h>

typedef struct
{
    ItemType_t  type;
    Vector3     position;
    Vector3     velocity;
    bool        active;
    bool        landed;
} ThrownItem_t;

/* Resets every slot to inactive. Not strictly required at startup (the
   backing array is zero-initialized, which already means inactive) */
void                items_init(void);

/* Applies gravity + motion to every active, not-yet-landed item, and flips
   landed=true the moment it touches the ground. Call once per frame. */
void                items_update(float deltatime);

/* Finds a free slot and activates it with the given type/position/velocity.
   Returns false (nothing spawned) if every slot is already in use. */
bool                items_spawn(ItemType_t type, Vector3 position, Vector3 velocity);

/* Read-only access to the full fixed-size array, for callers that need to
   scan every item (the flee-trigger check, the draw loop). out_count is
   always set to the array's fixed capacity - callers must still check
   .active themselves, same as iterating thrown_items used to work. */
const ThrownItem_t * items_get_all(int * out_count);

/* Deactivates the first active item within radius of position, if any.
   Used when an enemy reaches its flee target and "picks up" the item. */
void                items_consume_near(Vector3 position, float radius);
