#include <stdio.h>
#include "raylib.h"
#include "raymath.h"

#include "core/player/player.h"
#include "core/enemies/enemy.h"

const int screenWidth = 1280;
const int screenHeight = 720;

int main(void)
{
    Player_t * pl = player_initialize("cuongbip");
    Enemy_t * e1 = enemy_initialize("hero");
    InitWindow(screenWidth, screenHeight, "Cam - C99 & Raylib");

    // Initialize 
    Vector3 pl_pos      = player_get_position(pl);
    Vector3 pl_dir      = player_get_direction(pl);
    float   pl_speed    = player_get_speed(pl);
    float   pl_rotation = player_get_rotation(pl);

    Vector3 e1_pos      = enemy_get_position(e1);
    float   e1_speed    = enemy_get_speed(e1);
    float   e1_rotation = enemy_get_rotation(e1);

    // Camera config 
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 5.0f, 6.0f };
    camera.target = pl_pos;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // (Spherical Coordinates)
    float cameraRadius = 8.0f; // Distance from camera to player
    float cameraAngleH = 0.0f; // (Yaw)
    float cameraAngleV = 0.3f; // (Pitch)

    // Lock mouse
    DisableCursor(); 
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        /* Handle mouse rotating camera */
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngleH -= mouseDelta.x * 0.003f;
        cameraAngleV += mouseDelta.y * 0.003f;

        if (cameraAngleV > 1.2f) cameraAngleV = 1.2f;
        if (cameraAngleV < 0.1f) cameraAngleV = 0.1f;

        // Calculate camera pos
        camera.position.x = pl_pos.x + cameraRadius * sinf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.z = pl_pos.z + cameraRadius * cosf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.y = pl_pos.y + cameraRadius * sinf(cameraAngleV);
        camera.target = pl_pos;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        forward.y = 0;
        forward = Vector3Normalize(forward);
        Vector3 right = { -forward.z, 0.0f, forward.x };

        /* Need to pass by pointer to synch between internal functions & main */
        Vector3 old_pos = pl_pos;

        player_update_general(pl, &pl_pos, &pl_rotation, deltaTime, forward, right);
        enemy_update_general(e1, &e1_pos, &pl_pos, deltaTime);
        player_update_collider(pl);
        enemy_update_collider(e1);

        /* Check collision */
        Vector3 movement = Vector3Subtract(pl_pos, old_pos);
        if (CheckCollisionBoxes(player_get_collider(pl), enemy_get_collider(e1))) {

            BoundingBox pl_box = player_get_collider(pl);
            BoundingBox e1_box = enemy_get_collider(e1);
            
            // Calculate overlap penetration
            float overlap_x = fminf(pl_box.max.x, e1_box.max.x) - fmaxf(pl_box.min.x, e1_box.min.x);
            float overlap_y = fminf(pl_box.max.y, e1_box.max.y) - fmaxf(pl_box.min.y, e1_box.min.y);
            float overlap_z = fminf(pl_box.max.z, e1_box.max.z) - fmaxf(pl_box.min.z, e1_box.min.z);

            Vector3 normal = {0};
            if (overlap_x < overlap_y && overlap_x < overlap_z) {
                normal.x = (pl_box.min.x < e1_box.min.x) ? -1.0f : 1.0f;
            } else if (overlap_y < overlap_z) {
                normal.y = (pl_box.min.y < e1_box.min.y) ? -1.0f : 1.0f;
            } else {
                normal.z = (pl_box.min.z < e1_box.min.z) ? -1.0f : 1.0f;
            }

            // Calculate slide vector
            float into_surface = Vector3DotProduct(movement, normal);
            if (into_surface < 0.0f) {
                movement = Vector3Subtract(movement, Vector3Scale(normal, into_surface));
            }
        }
        
        /* Update current position after resolve sliding impact */
        pl_pos = Vector3Add(old_pos, movement);
        player_set_position(pl, pl_pos);
        
        BeginDrawing();
            ClearBackground(DARKGRAY);

            BeginMode3D(camera);
                DrawGrid(50, 1.0f);

                DrawCube(e1_pos, 2.0f, 2.0f, 2.0f, RED);

                DrawCylinderEx(pl_pos, Vector3Add(pl_pos, (Vector3){0, 2.0f, 0}), 0.6f, 0.6f, 16, BLUE);
                Vector3 lookAtDir = { sinf(pl_rotation * DEG2RAD), 1.0f, cosf(pl_rotation * DEG2RAD) };
                DrawSphere(Vector3Add(pl_pos, lookAtDir), 0.2f, GOLD);

            EndMode3D();

            // UI overrides
            DrawFPS(10, 10);
            DrawText("Mouse to rotate CAMERA | WASD to move", 10, 40, 20, RAYWHITE);
        EndDrawing();
    }
    player_destroy(pl);
    CloseWindow();
    return 0;
}
