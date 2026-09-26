#include <raylib.h>
#include "ui.h"
#include "../../physics/geometry.h"
#include "raymath.h"

void display_hp_bar(int x, int y, int width, int height, float hp, float max_hp)
{
    if (max_hp == 0) return;

    Color color;
    float ratio = hp / max_hp;
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    
    if (ratio >= 0.6) {
        color = GREEN;
    } else if (ratio > 0.3 && ratio < 0.6) {
        color = GOLD;
    } else {
        color = RED;
    } 

    DrawRectangle(x, y, width, height, DARKGRAY);
    DrawRectangle(x, y, (int)(width * ratio), height, color);
    DrawRectangleLines(x, y, width, height, BLACK);
}

void set_enemies_hpbar_position(
    Enemy_t ** enemy_list,
    Vector3 * enemy_positions,
    Camera3D camera,
    int count,
    int * bar_coord_x,
    int * bar_coord_y
)
{
    if (!enemy_list || !enemy_positions || !bar_coord_x || !bar_coord_y) return;
 
    CapsuleCollider3D_t * enemy_boxes[count];
    float head_offset[count];
    Vector3 top_head[count];
    Vector2 screen_pos[count];
    
    for (int i = 0; i < count; ++i) {
        if (!enemy_list[i]) continue;

        enemy_boxes[i] = enemy_get_collider(enemy_list[i]);
        head_offset[i] = geometry_capsule_get_coord(enemy_boxes[i], 1).y + geometry_capsule_get_radius(enemy_boxes[i]);
        top_head[i] = Vector3Add(enemy_positions[i], (Vector3){0.0f, head_offset[i] + 0.3f, 0.0f});
        screen_pos[i] = GetWorldToScreen(top_head[i], camera);
        bar_coord_x[i] = screen_pos[i].x - (float)ENEMY_HPBAR_WIDTH / 2;
        bar_coord_y[i] = screen_pos[i].y;
    }
}
