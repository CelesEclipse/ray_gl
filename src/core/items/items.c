#include "items.h"
#include "raymath.h"

#define MAX_THROWN_ITEMS    8
#define GRAVITY              9.8f
#define GROUND_Y              0.0f

static ThrownItem_t g_items[MAX_THROWN_ITEMS] = {0};

void items_init(void)
{
    for (int i = 0; i < MAX_THROWN_ITEMS; ++i) {
        g_items[i] = (ThrownItem_t){0};
    }
}

void items_update(float deltatime)
{
    for (int i = 0; i < MAX_THROWN_ITEMS; ++i) {
        if (!g_items[i].active || g_items[i].landed) continue;

        g_items[i].velocity.y -= GRAVITY * deltatime;
        g_items[i].position = Vector3Add(g_items[i].position,
            Vector3Scale(g_items[i].velocity, deltatime));

        if (g_items[i].position.y <= GROUND_Y) {
            g_items[i].position.y = GROUND_Y;
            g_items[i].landed = true;
            g_items[i].velocity = (Vector3){0};
        }
    }
}

bool items_spawn(ItemType_t type, Vector3 position, Vector3 velocity)
{
    for (int i = 0; i < MAX_THROWN_ITEMS; ++i) {
        if (!g_items[i].active) {
            g_items[i] = (ThrownItem_t){ type, position, velocity, true, false };
            return true;
        }
    }
    return false; // every slot in use
}

const ThrownItem_t * items_get_all(int * out_count)
{
    if (out_count) *out_count = MAX_THROWN_ITEMS;
    return g_items;
}

void items_consume_near(Vector3 position, float radius)
{
    for (int i = 0; i < MAX_THROWN_ITEMS; ++i) {
        if (g_items[i].active && Vector3Distance(g_items[i].position, position) < radius) {
            g_items[i].active = false;
            return; // first match only - matches old main.c behavior
        }
    }
}
