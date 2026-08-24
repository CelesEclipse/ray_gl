#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#include "core/player/player.h"
#include "core/enemies/enemy.h"
#include "core/hud/ui.h"
#include "core/collision/collision.h"

const int screenWidth = 1280;
const int screenHeight = 720;
#define     PLAYER_MAXHP    100
#define     ENEMY_MAXHP     120

int main(void)
{
    Player_t * pl = player_initialize("cuongbip");
    Enemy_t * e1 = enemy_initialize("hero");

    InitWindow(screenWidth, screenHeight, "Cam - C99 & Raylib");

    Vector3 pl_pos = player_get_position(pl);
    float pl_rotation = player_get_rotation(pl);

    Vector3 e1_pos = enemy_get_position(e1);

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

        Vector3 old_pos = pl_pos;

        /* Player movement */
        Vector3 movement = player_update_general(pl, &pl_rotation, deltaTime, forward, right);

        /* Enemy */
        Vector3 enemy_movement = enemy_update_general(e1, pl_pos, deltaTime);

        e1_pos = Vector3Add(e1_pos, enemy_movement);
        enemy_set_position(e1, e1_pos);

        /* Update colliders after movement */
        player_update_collider(pl);
        enemy_update_collider(e1);

        // AABB 
        CollisionResult_t col = collision_resolve_aabb(
            player_get_collider(pl),
            enemy_get_collider(e1),
            movement
        );
        pl_pos = Vector3Add(pl_pos, col.c_movement);
        pl_pos = Vector3Add(pl_pos, col.c_correction);
        player_set_position(pl, pl_pos);

        /* render */
        BoundingBox e1_box = enemy_get_collider(e1);
        float head_offset = e1_box.max.y - e1_pos.y;
        Vector3 e1_top_head = Vector3Add(e1_pos, (Vector3){0.0f, head_offset + 0.3f, 0.0f});
        Vector2 e1_screen_pos = GetWorldToScreen(e1_top_head, camera);
        int e1_bar_width = 100;
        BeginDrawing();
            ClearBackground(DARKGRAY);
            BeginMode3D(camera);
                DrawGrid(50, 1.0f);
                DrawCube(e1_pos, 2.0f, 2.0f, 2.0f, GREEN);

                DrawCylinderEx(pl_pos, Vector3Add(pl_pos, (Vector3){0, 2.0f, 0}), 0.6f, 0.6f, 16, BLUE);
                Vector3 lookAtDir = {sinf(pl_rotation * DEG2RAD), 1.0f, cosf(pl_rotation * DEG2RAD)};
                DrawSphere(Vector3Add(pl_pos, lookAtDir), 0.2f, GOLD);

            EndMode3D();

            DrawFPS(10, 10);
            display_hp_bar(10, 40, 200, 20, player_get_hp(pl), PLAYER_MAXHP);
            display_hp_bar(e1_screen_pos.x - e1_bar_width / 2,
                            e1_screen_pos.y,
                            e1_bar_width, 15, enemy_get_hp(e1), ENEMY_MAXHP);

            DrawText(TextFormat("HP: %.0f", player_get_hp(pl)), 15, 45, 10, RAYWHITE);

        EndDrawing();
    }

    player_destroy(pl);
    enemy_destroy(e1);
    CloseWindow();

    return 0;
}
