#include <stdlib.h>
#include <time.h>
#include "core/models/model_hdl.h"
#include "physics/geometry.h"
#include "raylib.h"
#include "raymath.h"

#include "core/player/player.h"
#include "core/enemies/enemy.h"
#include "core/hud/ui.h"
#include "core/collision/collision.h"
#include "utils/utils.h"

#define     MODEL_PATH      "../assets/raw_models/pl_model.glb"

const int screenWidth = 1280;
const int screenHeight = 720;
#define     PLAYER_MAXHP    100
#define     ENEMY_MAXHP     120
#define     DEBUG_KEY       0
#define     MODEL_LOAD      1
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

    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_list[i] = enemy_initialize("hero");
        enpos_list[i] = generate_random_vector(min_random_val, max_random_val);
    }

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

#if MODEL_LOAD
    Model pl_model = LoadModel(MODEL_PATH);
    for (int i = 0; i < pl_model.materialCount; i++) {
        pl_model.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    }
    pl_model.transform = MatrixMultiply(pl_model.transform, MatrixRotateX(-90.0f * DEG2RAD));
#endif

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_C)) {
            show_circle = !show_circle;
        }

        /* Camera input */
        Vector2 mouseDelta = GetMouseDelta();
        cameraAngleH -= mouseDelta.x * 0.003f;
        cameraAngleV += mouseDelta.y * 0.003f;

        if (cameraAngleV > 1.2f) cameraAngleV = 1.2f;
        if (cameraAngleV < 0.1f) cameraAngleV = 0.1f;

        camera.position.x = pl_pos.x + cameraRadius * sinf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.z = pl_pos.z + cameraRadius * cosf(cameraAngleH) * cosf(cameraAngleV);
        camera.position.y = pl_pos.y + cameraRadius * sinf(cameraAngleV);
        camera.target = pl_pos;

        Vector3 forward = Vector3Normalize(Vector3Subtract(camera.target, camera.position));
        forward.y = 0.0f;
        forward = Vector3Normalize(forward);
        Vector3 right = {-forward.z, 0.0f, forward.x};

        /* ================= UPDATE ================= */

        // 1. decide intent (no positions changed yet)
        Vector3 movement = player_update_general(pl, &pl_rotation, deltaTime, forward, right);
        player_normal_attack(pl, deltaTime);

        Vector3 enemy_movement[ENEMY_NUM];
        for (int i = 0; i < ENEMY_NUM; ++i) {
            enemy_movement[i] = enemy_update_general(enemy_list[i], pl_pos, deltaTime);
            enemy_normal_attack(enemy_list[i], deltaTime);
        }

        // 2: apply enemy movement + refresh enemy colliders
        for (int i = 0; i < ENEMY_NUM; ++i) {
            enpos_list[i] = Vector3Add(enpos_list[i], enemy_movement[i]);
            enemy_set_position(enemy_list[i], enpos_list[i]);
            enemy_update_collider(enemy_list[i]);
        }

        // 3: snapshot player collider BEFORE resolving anything this frame
        player_update_collider(pl);
        CapsuleCollider3D_t * player_collider_snapshot = player_get_collider(pl);
        Vector3 pl_correction_total = {0};

        // 4: physical collision (player body vs each enemy BODY, not hitbox)
        for (int i = 0; i < ENEMY_NUM; ++i) {
            CollisionResult_Capsule_t body_col = collision_resolve_capsule_box(
                player_collider_snapshot,
                enemy_get_collider(enemy_list[i])
            );
            if (body_col.penetration_depth > 0) {
                Vector3 push = Vector3Scale(body_col.normal_vector, body_col.penetration_depth);
                pl_correction_total = Vector3Add(pl_correction_total, push);
            }
        }

        // 5: combat (separate from physical collision, it's hitbox time here)
        for (int i = 0; i < ENEMY_NUM; ++i) {
            if (enemy_get_did_attack(enemy_list[i])) {
                CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                    player_collider_snapshot,
                    enemy_get_hitbox(enemy_list[i], pl_pos)
                );
                if (hit.penetration_depth > 0) {
                    player_take_damage(pl, 10.0f);
                }
            }

            if (player_get_did_attack(pl)) {
                if (collision_check_hitbox(player_get_hitbox(pl), enemy_get_collider(enemy_list[i]))) {
                    enemy_take_damage(enemy_list[i], 10.0f);
                }
            }
        }

        // 6: apply player movement + correction once
        pl_pos = Vector3Add(pl_pos, movement);
        pl_pos = Vector3Add(pl_pos, pl_correction_total);
        player_set_position(pl, pl_pos);
        player_update_collider(pl);

        // 7: HUD display
        BoundingBox enbox_list[ENEMY_NUM];
        float head_offset[ENEMY_NUM];
        Vector3 en_top_head[ENEMY_NUM];
        Vector2 en_screen_pos[ENEMY_NUM];
        int en_bar_x[ENEMY_NUM];
        int en_bar_y[ENEMY_NUM];
        int e1_bar_width = 100;

        for (int i = 0; i < ENEMY_NUM; ++i) {
            enbox_list[i] = enemy_get_collider(enemy_list[i]);
            head_offset[i] = enbox_list[i].max.y - enpos_list[i].y;
            en_top_head[i] = Vector3Add(enpos_list[i], (Vector3){0.0f, head_offset[i] + 0.3f, 0.0f});
            en_screen_pos[i] = GetWorldToScreen(en_top_head[i], camera);
            en_bar_x[i] = en_screen_pos[i].x - e1_bar_width / 2;
            en_bar_y[i] = en_screen_pos[i].y;
        }

        /* render */
        BoundingBox model_box = model_get_scaled_bbox(pl_model, pl_pos, 150.0f, 0.5f);
        BeginDrawing();
            ClearBackground(DARKGRAY);
            BeginMode3D(camera);

                if (show_circle) {
#if MODEL_LOAD
                    DrawCapsuleWires(
                        geometry_capsule_get_coord(player_get_collider(pl), 0),
                        geometry_capsule_get_coord(player_get_collider(pl), 1),
                        geometry_capsule_get_radius(player_get_collider(pl)),
                        8, 8, (Color){64, 224, 208, 255}
                    );
#else
                    DrawBoundingBox(player_get_hitbox(pl), RED);
#endif
                    for (int i = 0; i < ENEMY_NUM; ++i) {
                        enemy_draw_detect_range(enemy_list[i]);
                        DrawBoundingBox(enemy_get_hitbox(enemy_list[i], pl_pos), RED);
                    }
                }
                DrawGrid(50, 1.0f);

                for (int i = 0; i < ENEMY_NUM; ++i) {
                    if (!enemy_is_dead(enemy_list[i])) {
                        DrawCube(enpos_list[i], 2.0f, 2.0f, 2.0f, GREEN);
                    }
                }
#if MODEL_LOAD
                Vector3 model_scale_vec = {150.0f, 150.0f, 150.0f};
                Vector3 rotation_axis = {0.0f, 1.0f, 0.0f};
                float facing_angle = pl_rotation;
                DrawModelEx(pl_model, pl_pos, rotation_axis, facing_angle, model_scale_vec, WHITE);
#else
                DrawCylinderEx(pl_pos, Vector3Add(pl_pos, (Vector3){0, 2.0f, 0}), 0.6f, 0.6f, 16, BLUE);
                Vector3 lookAtDir = {sinf(pl_rotation * DEG2RAD), 1.0f, cosf(pl_rotation * DEG2RAD)};
                DrawSphere(Vector3Add(pl_pos, lookAtDir), 0.2f, GOLD);
#endif

            EndMode3D();

            DrawFPS(10, 10);
            display_hp_bar(10, 40, 200, 20, player_get_hp(pl), PLAYER_MAXHP);

            for (int i = 0; i < ENEMY_NUM; ++i) {
                if (!enemy_is_dead(enemy_list[i])) {
                    display_hp_bar(en_bar_x[i], en_bar_y[i], e1_bar_width, 15, enemy_get_hp(enemy_list[i]), ENEMY_MAXHP);
                }
            }
            DrawText(TextFormat("HP: %.0f", player_get_hp(pl)), 15, 45, 10, RAYWHITE);
            DrawText(TextFormat("pl : %s", state_to_string(player_get_state(pl))), 15, 85, 30, DARKBLUE);

            if (player_is_dead(pl)) {
                DrawText("YOU DIED", screenWidth/2 - 100, screenHeight/2, 40, RED);
            }

        EndDrawing();
    }

#if MODEL_LOAD
    UnloadModel(pl_model);
#endif

    player_destroy(pl);
    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_destroy(enemy_list[i]);
    }

    CloseWindow();
    return 0;
}
