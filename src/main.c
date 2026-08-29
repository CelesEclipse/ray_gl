#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#include "core/player/player.h"
#include "core/enemies/enemy.h"
#include "core/hud/ui.h"
#include "core/collision/collision.h"
#include "utils/utils.h"

const int screenWidth = 1280;
const int screenHeight = 720;
#define     PLAYER_MAXHP    100
#define     ENEMY_MAXHP     120
#define     DEBUG_KEY       0
#define     ENEMY_NUM       5

static Vector3 generate_random_vector(float min, float max)
{
    Vector3 v;
    v.x = random_float(min, max);
    v.y = 0.0f;
    v.z = random_float(min, max);
    return v;
}

int main(void)
{
    srand((unsigned int)time(NULL));
    InitWindow(screenWidth, screenHeight, "Cam - C99 & Raylib");
    bool show_circle = false;
    float min_random_val = -10.0f;
    float max_random_val = 10.0f;

    Player_t * pl = player_initialize("cuongbip");
    Enemy_t * enemy_list[ENEMY_NUM];
    Vector3 enpos_list[ENEMY_NUM];

    // Random positions for enemies
    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_list[i] = enemy_initialize("hero");
        enpos_list[i] = generate_random_vector(min_random_val, max_random_val);
    }
    // Enemy_t * e1 = enemy_initialize("hero");
    Vector3 pl_pos = player_get_position(pl);
    float pl_rotation = player_get_rotation(pl);

    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 5.0f, 6.0f};
    camera.target = pl_pos;
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraRadius = 8.0f;
    float cameraAngleH = 0.0f;
    float cameraAngleV = 0.3f;

    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

    #if DEBUG_KEY
        /* test the bar only */
        // Use IsKeyPressed instead of IsKeyDown (which can take 2-3 frames)
        // so it will triply decrease the amount of the passed argument
        if (IsKeyPressed(KEY_Q)) {
            player_set_hp(pl, -10.0f);
        }
        if (IsKeyPressed(KEY_R)) {
            enemy_take_damage(e1, 10.0f);
        }
        
        // Respawn/despawn or true reset later, until main menu progress
        if (player_is_dead(pl) && enemy_is_dead(e1)) {
            if (IsKeyPressed(KEY_LEFT_CONTROL)) {
                // player_reset
                // enemy_reset
            }
        }
    #endif

        // Flip circle for simple enemy detection
        // Move this out of render context
        if (IsKeyPressed(KEY_C)) {
            show_circle = !show_circle;
        }

        /* Camera input */
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngleH -= mouseDelta.x * 0.003f;
        cameraAngleV += mouseDelta.y * 0.003f;

        if (cameraAngleV > 1.2f) cameraAngleV = 1.2f;
        if (cameraAngleV < 0.1f) cameraAngleV = 0.1f;

        /* Camera position */
        camera.position.x = pl_pos.x + cameraRadius * sinf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.z = pl_pos.z + cameraRadius * cosf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.y = pl_pos.y + cameraRadius * sinf(cameraAngleV);
        camera.target = pl_pos;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));

        forward.y = 0.0f;
        forward = Vector3Normalize(forward);
        Vector3 right = {-forward.z, 0.0f, forward.x};

        /* ================= UPDATE ================= */

        /* Player movement */
        Vector3 movement = player_update_general(pl, &pl_rotation, deltaTime, forward, right);

        /* Enemy */
        Vector3 enemy_movement[ENEMY_NUM];
        player_normal_attack(pl, deltaTime);
        BoundingBox enbox_list[ENEMY_NUM];
        float head_offset[ENEMY_NUM];
        Vector3 en_top_head[ENEMY_NUM];
        Vector2 en_screen_pos[ENEMY_NUM];
        int en_bar_x[ENEMY_NUM];
        int en_bar_y[ENEMY_NUM];
        int e1_bar_width = 100;

        for (int i = 0; i < ENEMY_NUM; ++i) {
            enemy_movement[i] = enemy_update_general(enemy_list[i], pl_pos, deltaTime);
            enemy_normal_attack(enemy_list[i], deltaTime);
            if (enemy_get_did_attack(enemy_list[i])) {
                if (collision_check_hitbox(
                    enemy_get_hitbox(enemy_list[i], player_get_position(pl)),
                    player_get_collider(pl)
                )) {
                    player_take_damage(pl, 10.0f);
                }
            }
        
            if (player_get_did_attack(pl)) {
                if (collision_check_hitbox(
                    player_get_hitbox(pl),
                    enemy_get_collider(enemy_list[i])
                )) {
                    enemy_take_damage(enemy_list[i], 10.0f);
                }
            }

            enpos_list[i] = Vector3Add(enpos_list[i], enemy_movement[i]);
            enemy_set_position(enemy_list[i], enpos_list[i]);

            /* Update colliders after movement */
            player_update_collider(pl);
            enemy_update_collider(enemy_list[i]);

            // AABB 
            CollisionResult_t col = collision_resolve_aabb(
                player_get_collider(pl),
                enemy_get_collider(enemy_list[i]),
                movement
            );
            pl_pos = Vector3Add(pl_pos, col.c_movement);
            pl_pos = Vector3Add(pl_pos, col.c_correction);
            player_set_position(pl, pl_pos);

            enbox_list[i] = enemy_get_collider(enemy_list[i]);
            head_offset[i] = enbox_list[i].max.y - enpos_list[i].y;
            en_top_head[i] = Vector3Add(enpos_list[i], (Vector3){0.0f, head_offset[i] + 0.3f, 0.0f});
            en_screen_pos[i] = GetWorldToScreen(en_top_head[i], camera);
            en_bar_x[i] = en_screen_pos[i].x - e1_bar_width / 2;
            en_bar_y[i] = en_screen_pos[i].y;
        }

        /* render */


        BeginDrawing();
            ClearBackground(DARKGRAY);
            BeginMode3D(camera);
                    
                if (show_circle) {
                    DrawBoundingBox(player_get_hitbox(pl), RED);
                    for (int i = 0; i < ENEMY_NUM; ++i) {
                        enemy_draw_detect_range(enemy_list[i]);
                        DrawBoundingBox(enemy_get_hitbox(enemy_list[i], pl_pos), RED);
                    }
                }
                DrawGrid(50, 1.0f);
                
                // Simple conditional render
                for (int i = 0; i < ENEMY_NUM; ++i) {
                    if (!enemy_is_dead(enemy_list[i])) {
                        DrawCube(enpos_list[i], 2.0f, 2.0f, 2.0f, GREEN);
                    }
                }
                
                DrawCylinderEx(pl_pos, Vector3Add(pl_pos, (Vector3){0, 2.0f, 0}), 0.6f, 0.6f, 16, BLUE);
                Vector3 lookAtDir = {sinf(pl_rotation * DEG2RAD), 1.0f, cosf(pl_rotation * DEG2RAD)};
                DrawSphere(Vector3Add(pl_pos, lookAtDir), 0.2f, GOLD);

            EndMode3D();

            DrawFPS(10, 10);
            display_hp_bar(10, 40, 200, 20, player_get_hp(pl), PLAYER_MAXHP);

            for (int i = 0; i < ENEMY_NUM; ++i) {
                display_hp_bar(en_bar_x[i], en_bar_y[i], e1_bar_width, 15, enemy_get_hp(enemy_list[i]), ENEMY_MAXHP);
            }
            DrawText(TextFormat("HP: %.0f", player_get_hp(pl)), 15, 45, 10, RAYWHITE);
            DrawText(TextFormat("pl : %s", state_to_string(player_get_state(pl))), 15, 85, 30, DARKBLUE);
            // DrawText(TextFormat("HP: %.0f", enemy_get_hp(e1)), en_bar_x + 5, en_bar_y + 5, 10, RAYWHITE);
            // DrawText(TextFormat("e1 : %s", state_to_string(enemy_get_state(e1))), 1000, 85, 30, PURPLE);
            // DrawText(TextFormat("E1 ATK: %.1f", enemy_get_atk_timer(e1)), 1000, 65, 15, PURPLE);
            if (player_is_dead(pl)) {
                DrawText("YOU DIED", screenWidth/2 - 100, screenHeight/2, 40, RED);
            }

        EndDrawing();
    }

    player_destroy(pl);
    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_destroy(enemy_list[i]);
    }
    
    CloseWindow();

    return 0;
}
