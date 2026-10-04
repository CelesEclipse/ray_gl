#include "core/models/animations/anim_controller.h"
#if defined (PLATFORM_DESKTOP)
    #define GLSL_VERSION    100
#else
    #define GLSL_VERSION    330
#endif

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "physics/geometry.h"
#include "raylib.h"
#include "raymath.h"

#include "core/core.h"
#include "common/common.h"
#include "core/camera/camera.h"
#include "core/player/player.h"
#include "core/enemies/enemy.h"
#include "core/hud/ui.h"
#include "core/items/items.h"
#include "utils/utils.h"

#define SUPPORT_GPU_SKINNING    1

#define     MODEL_PATH      "../assets/working_assets/xbot72.glb"
#define     SKINNING_VS     "../assets/shaders/glsl330/skinning.vs"
#define     SKINNING_FS     "../assets/shaders/glsl330/skinning.fs"

#define     SCREENWIDTH     1280
#define     SCREENHEIGHT    720
#define     PLAYER_MAXHP    100
#define     ENEMY_MAXHP     120
#define     DEBUG_KEY       0
#define     ENEMY_NUM       5

#define ITEM_FLEE_TRIGGER_RADIUS   6.0f
#define THROW_RELEASE_FRAME        38

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
    InitWindow(SCREENWIDTH, SCREENHEIGHT, "Cam - C99 & Raylib");
    bool show_circle = false;
    float min_random_val = -15.0f;
    float max_random_val = 15.0f;

    Player_t * pl = player_initialize("cuongbip");
    Enemy_t * enemy_list[ENEMY_NUM];
    Vector3 enpos_list[ENEMY_NUM];
    Model enemy_models[ENEMY_NUM];

    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_list[i] = enemy_initialize("hero");
        enpos_list[i] = generate_random_vector(min_random_val, max_random_val);
    }

    Vector3 pl_pos = player_get_position(pl);
    float pl_rotation = player_get_rotation(pl);

    Camera3D camera = {0};
    camera.position = (Vector3){0.0f, 5.0f, 6.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    float cameraAngleH = 0.0f;
    float cameraAngleV = 0.3f;

    /* Camera input */
    Vector3 forward, right;

    DisableCursor();
    SetTargetFPS(60);

    Model pl_model = LoadModel(MODEL_PATH);

#if SUPPORT_GPU_SKINNING
    Shader skinning_shader = LoadShader(SKINNING_VS, SKINNING_FS);
    TraceLog(LOG_INFO, "skinning shader id = %u", skinning_shader.id);
    for (int i = 0; i < pl_model.materialCount; ++i) {
        pl_model.materials[i].shader = skinning_shader;
    }
#endif

    for (int i = 0; i < pl_model.materialCount; i++) {
        pl_model.materials[i].maps[MATERIAL_MAP_DIFFUSE].color = WHITE;
    }
    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_models[i] = LoadModel(MODEL_PATH);
        for (int j = 0; j < enemy_models[i].materialCount; ++j) {
            enemy_models[i].materials[j].maps[MATERIAL_MAP_DIFFUSE].color = GREEN;
        }
    }
    
    int anim_count = 0, idleidx = 0, walkidx = 0, runidx = 0, deathidx = 0, atkidx = 0, atk360idx = 0, throwidx = 0;
    ModelAnimation * animations = LoadModelAnimations(MODEL_PATH, &anim_count);
    for (int i = 0; i < anim_count; ++i) {
        if (strstr(animations[i].name, "Idle"))         idleidx = i;
        if (strstr(animations[i].name, "Walk"))         walkidx = i;
        if (strstr(animations[i].name, "Run"))          runidx = i;
        if (strstr(animations[i].name, "Death"))        deathidx = i;
        if (strstr(animations[i].name, "quickatk"))     atkidx = i;
        if (strstr(animations[i].name, "strongatk360")) atk360idx = i;
        if (strstr(animations[i].name, "Throw"))        throwidx = i;
    }

    int anim_frame = 0;
    bool pl_attacking = false;
    bool pl_throwing = false;
    Vector3 pl_throw_spawn_pos = {0};
    Vector3 pl_throw_spawn_vel = {0};
    AnimState_t pl_current_atk_anim = ANIM_IDLE;

    /* Main loop */
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        camera_update(&camera, &cameraAngleH, &cameraAngleV, pl_pos);
        camera_get_basis(camera, &forward, &right);

        if (IsKeyPressed(KEY_C)) {
            show_circle = !show_circle;
        }

        /* ================= UPDATE ================= */

        // 1. decide intent (no positions changed yet)
        AttackRequest_t req = ATK_NONE;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) req = ATK_QUICK;
        if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) req = ATK_360;
        if (!pl_throwing && IsKeyPressed(KEY_Q)) {
            if (player_throw_items(pl, ITEM_COIN, &pl_throw_spawn_pos, &pl_throw_spawn_vel)) {
                pl_throwing = true;
                anim_frame = 0;
                player_set_anim(pl, ANIM_THROW);
            }
        }

        items_update(deltaTime);

        Vector3 movement = player_update_general(pl, &pl_rotation, deltaTime, forward, right);
        
        bool sprinting = IsKeyDown(KEY_LEFT_SHIFT);
        player_set_sprint(pl, sprinting);
        player_normal_attack(pl, deltaTime, req);    // Bug note later

        if (!player_is_dead(pl)) {
            if (!pl_attacking && player_get_did_attack(pl)) {
                pl_attacking = true;
                anim_frame = 0;
                pl_current_atk_anim = (player_get_last_atk(pl) == ATK_360) ? ANIM_ATK_360 : ANIM_ATK;
            }
            if (pl_attacking) {
                player_set_anim(pl, pl_current_atk_anim);
            } else if (pl_throwing) {
                player_set_anim(pl, ANIM_THROW);
            } else {
                player_set_anim(pl, (player_get_state(pl) != P_MOVING) ? ANIM_IDLE : (sprinting ? ANIM_RUN : ANIM_WALK));
            }
        } else {
            player_set_anim(pl, ANIM_DEATH);
        }
        
        AnimClipSet_t clips = { idleidx, walkidx, runidx, deathidx, atkidx, atk360idx, throwidx };
        int clip = anim_controller_resolve_clip(player_get_anim_current(pl), clips);

        /* Only advance the clip while the player is actually moving (WASD held).
           When idle, hold on frame 0 instead of freezing mid-stride.
           Check for death
        */
        bool pl_atk_finished = false;
        anim_frame = anim_controller_advance(player_get_anim_current(pl), anim_frame, animations[clip].keyframeCount, &pl_atk_finished);
        if (pl_atk_finished) {
            pl_attacking = false;
            pl_throwing = false;
        }

        // spawn the actual thrown item on its release frame, not on keypress
        if (pl_throwing && player_get_anim_current(pl) == ANIM_THROW && anim_frame == THROW_RELEASE_FRAME) {
            items_spawn(ITEM_COIN, pl_throw_spawn_pos, pl_throw_spawn_vel);
        }

        UpdateModelAnimation(pl_model, animations[clip], anim_frame);

        /* enemy */
        Vector3 enemy_movement[ENEMY_NUM];
        for (int i = 0; i < ENEMY_NUM; ++i) {

            // Trigger: only from idle or actively chasing - never interrupt an attack, death, or an already-fleeing enemy
            if (enemy_get_state(enemy_list[i]) == E_IDLE || enemy_get_state(enemy_list[i]) == E_MOVING) {
                int item_count = 0;
                const ThrownItem_t * items = items_get_all(&item_count);
                for (int t = 0; t < item_count; ++t) {
                    if (items[t].active && items[t].landed &&
                        Vector3Distance(enpos_list[i], items[t].position) < ITEM_FLEE_TRIGGER_RADIUS) {
                        enemy_start_flee(enemy_list[i], items[t].position);
                        break;
                    }
                }
            }

            int clip = 0;
            enemy_movement[i] = enemy_tick(enemy_list[i], pl_pos, deltaTime, clips, animations, &clip);

            if (enemy_flee_did_reach_target(enemy_list[i])) {
                items_consume_near(enpos_list[i], FLEE_PICKUP_RADIUS);
            }

            UpdateModelAnimation(enemy_models[i], animations[clip], enemy_get_anim_frame(enemy_list[i]));
        }

        // 2: apply enemy movement + refresh enemy colliders
        for (int i = 0; i < ENEMY_NUM; ++i) {
            enpos_list[i] = Vector3Add(enpos_list[i], enemy_movement[i]);
            enemy_set_position(enemy_list[i], enpos_list[i]);
            enemy_update_collider(enemy_list[i]);
        }

        // 3: combat resolve
        Vector3 pl_correction_total = {0};
        combat_resolve(pl, enemy_list, ENEMY_NUM, pl_pos, anim_frame, &pl_correction_total);

        // 6: apply player movement + correction once
        pl_pos = Vector3Add(pl_pos, movement);
        pl_pos = Vector3Add(pl_pos, pl_correction_total);
        player_set_position(pl, pl_pos);
        player_update_collider(pl);

        // 7: HUD display
        int en_bar_x[ENEMY_NUM];
        int en_bar_y[ENEMY_NUM];
        set_enemies_hpbar_position(enemy_list, enpos_list, camera, ENEMY_NUM, en_bar_x, en_bar_y);

        /* render */
        BeginDrawing();
            ClearBackground(DARKGRAY);
            BeginMode3D(camera);
                
                int draw_item_count = 0;
                const ThrownItem_t * draw_items = items_get_all(&draw_item_count);
                for (int t = 0; t < draw_item_count; ++t) {
                    if (draw_items[t].active) {
                        DrawCylinder(draw_items[t].position, 0.3f, 0.3f, 0.08f, 16, GOLD);
                        DrawCylinderWires(draw_items[t].position, 0.3f, 0.3f, 0.08f, 16, BLACK);
                    }
                }
                if (show_circle) {
                    DrawCapsuleWires(
                        geometry_capsule_get_coord(player_get_collider(pl), 0),
                        geometry_capsule_get_coord(player_get_collider(pl), 1),
                        geometry_capsule_get_radius(player_get_collider(pl)),
                        8, 8, (Color){64, 224, 208, 255}
                    );
                    for (int i = 0; i < ENEMY_NUM; ++i) {
                        enemy_draw_detect_range(enemy_list[i]);
                        DrawCapsuleWires(
                            geometry_capsule_get_coord(enemy_get_collider(enemy_list[i]), 0),
                            geometry_capsule_get_coord(enemy_get_collider(enemy_list[i]), 1),
                            geometry_capsule_get_radius(enemy_get_collider(enemy_list[i])),
                            8, 8, ORANGE
                        );
                    }
                    for (int t = 0; t < draw_item_count; ++t) {
                        if (draw_items[t].active) {
                            DrawCircle3D(draw_items[t].position, ITEM_FLEE_TRIGGER_RADIUS, (Vector3){1,0,0}, 90.0f, YELLOW);
                        }
                    }
                }
                DrawGrid(50, 1.0f);

                Vector3 model_scale_vec = {1.0f, 1.0f, 1.0f};
                Vector3 rotation_axis = {0.0f, 1.0f, 0.0f}; /* yaw is a rotation about Y, not a zero vector */
                float facing_angle = player_get_rotation(pl);
                DrawModelEx(pl_model, pl_pos, rotation_axis, facing_angle, model_scale_vec, WHITE);

                for (int i = 0; i < ENEMY_NUM; ++i) {                        
                        float enemy_facing_angle = enemy_get_rotation(enemy_list[i]);
                        DrawModelEx(enemy_models[i], enpos_list[i], rotation_axis, enemy_facing_angle, model_scale_vec, ORANGE);
                }

            EndMode3D();

            DrawFPS(10, 10);
            display_hp_bar(10, 40, 200, 20, player_get_hp(pl), PLAYER_MAXHP);

            for (int i = 0; i < ENEMY_NUM; ++i) {
                if (!enemy_is_dead(enemy_list[i])) {
                    display_hp_bar(en_bar_x[i], en_bar_y[i], ENEMY_HPBAR_WIDTH, 15, enemy_get_hp(enemy_list[i]), ENEMY_MAXHP);
                }
            }
            DrawText(TextFormat("HP: %.0f", player_get_hp(pl)), 15, 45, 10, RAYWHITE);
            DrawText(TextFormat("pl : %s", state_to_string(player_get_state(pl))), 15, 85, 30, DARKBLUE);

            if (player_is_dead(pl)) {
                DrawText("YOU DIED", SCREENWIDTH/2 - 100, SCREENHEIGHT/2, 40, RED);
            }

        EndDrawing();
    }
    
#if SUPPORT_GPU_SKINNING
    UnloadShader(skinning_shader);
#endif
    UnloadModelAnimations(animations, anim_count);
    UnloadModel(pl_model);
    for (int i = 0; i < ENEMY_NUM; ++i) {
        UnloadModel(enemy_models[i]);
    }

    player_destroy(pl);
    for (int i = 0; i < ENEMY_NUM; ++i) {
        enemy_destroy(enemy_list[i]);
    }

    CloseWindow();
    return 0;
}
