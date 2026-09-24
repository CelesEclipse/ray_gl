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

#include "common/common.h"
#include "core/player/player.h"
#include "core/enemies/enemy.h"
#include "core/hud/ui.h"
#include "core/collision/collision.h"
#include "utils/utils.h"

#define SUPPORT_GPU_SKINNING    1

#define     MODEL_PATH      "../assets/working_assets/xbot71.glb"
#define     SKINNING_VS     "../assets/shaders/glsl330/skinning.vs"
#define     SKINNING_FS     "../assets/shaders/glsl330/skinning.fs"

const int screenWidth = 1280;
const int screenHeight = 720;
#define     PLAYER_MAXHP    100
#define     ENEMY_MAXHP     120
#define     DEBUG_KEY       0
#define     ENEMY_NUM       5
#define     ENEMY_SPRINT_DIST_ENTER   7.0f
#define     ENEMY_SPRINT_DIST_EXIT    5.0f
#define     ATK_IMPACT_FRAME            12

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
    Model enemy_models[ENEMY_NUM];

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
    
    int anim_count = 0, idleidx = 0, walkidx = 0, runidx = 0, deathidx = 0, atkidx = 0, atk360idx = 0;
    ModelAnimation * animations = LoadModelAnimations(MODEL_PATH, &anim_count);
    for (int i = 0; i < anim_count; ++i) {
        if (strstr(animations[i].name, "Idle"))         idleidx = i;
        if (strstr(animations[i].name, "Walk"))         walkidx = i;
        if (strstr(animations[i].name, "Run"))          runidx = i;
        if (strstr(animations[i].name, "Death"))        deathidx = i;
        if (strstr(animations[i].name, "quickatk"))     atkidx = i;
        if (strstr(animations[i].name, "strongatk360")) atk360idx = i;
    }

    int anim_frame = 0;
    bool pl_attacking = false;
    bool en_attacking[ENEMY_NUM] = {false};

    /* Main loop */
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
        
        bool sprinting = IsKeyDown(KEY_LEFT_SHIFT);
        player_set_sprint(pl, sprinting);
        player_normal_attack(pl, deltaTime);    // Bug note later

        if (!player_is_dead(pl)) {
            if (!pl_attacking && player_get_did_attack(pl)) {
                pl_attacking = true;
                anim_frame = 0;
            }
            player_set_anim(pl, pl_attacking ? ANIM_ATK
                : (player_get_state(pl) != P_MOVING) ? ANIM_IDLE
                : (sprinting ? ANIM_RUN : ANIM_WALK));
        } else {
            player_set_anim(pl, ANIM_DEATH);
        }
        
        int clip = idleidx;
        switch (player_get_anim_current(pl)) {
            case ANIM_WALK:     clip = walkidx;     break;
            case ANIM_RUN:      clip = runidx;      break;
            case ANIM_DEATH:    clip = deathidx;    break;
            case ANIM_ATK:      clip = atkidx;      break;
            case ANIM_ATK_360:  clip = atk360idx;   break;
            case ANIM_IDLE:
            default:            clip = idleidx;     break;
        }

        /* Only advance the clip while the player is actually moving (WASD held).
           When idle, hold on frame 0 instead of freezing mid-stride.
           Check for death
        */
        if (player_get_anim_current(pl) == ANIM_DEATH) {
            if (anim_frame < animations[clip].keyframeCount - 1) anim_frame++;
        } else if (player_get_anim_current(pl) == ANIM_ATK) {
            if (anim_frame < animations[clip].keyframeCount - 1) {
                anim_frame++;
            } else {
                pl_attacking = false;
            }
        } else if (player_get_anim_current(pl) != ANIM_IDLE) {
            anim_frame = (anim_frame + 1) % animations[clip].keyframeCount;
        } else {
            anim_frame = 0;
        }

        UpdateModelAnimation(pl_model, animations[clip], anim_frame);

        /* enemy */
        Vector3 enemy_movement[ENEMY_NUM];
        for (int i = 0; i < ENEMY_NUM; ++i) {
            float dist_to_pl = Vector3Distance(pl_pos, enpos_list[i]);
            bool was_sprinting = enemy_get_speed(enemy_list[i]) > 10.0f;
            bool en_sprinting = was_sprinting
                ? (dist_to_pl > ENEMY_SPRINT_DIST_EXIT)
                : (dist_to_pl > ENEMY_SPRINT_DIST_ENTER);
            enemy_set_sprint(enemy_list[i], en_sprinting);

            enemy_movement[i] = enemy_update_general(enemy_list[i], pl_pos, deltaTime);
            enemy_normal_attack(enemy_list[i], deltaTime);

            // Same as player but array
            if (!enemy_is_dead(enemy_list[i])) {
                if (!en_attacking[i] && enemy_get_did_attack(enemy_list[i])) {
                    en_attacking[i] = true;
                    enemy_set_anim_frame(enemy_list[i], 0);
                }
                enemy_set_anim(enemy_list[i], en_attacking[i] ? ANIM_ATK
                    : (enemy_get_state(enemy_list[i]) == E_MOVING)
                    ? (en_sprinting ? ANIM_RUN : ANIM_WALK)
                    : ANIM_IDLE);
            } else {
                enemy_set_anim(enemy_list[i], ANIM_DEATH);
            }

            int clip = idleidx;
            switch (enemy_get_anim_current(enemy_list[i])) {
                case ANIM_WALK:     clip = walkidx;     break;
                case ANIM_RUN:      clip = runidx;      break;
                case ANIM_DEATH:    clip = deathidx;    break;
                case ANIM_ATK:      clip = atkidx;      break;
                case ANIM_ATK_360:  clip = atk360idx;   break;
                default:            clip = idleidx;     break;
            }

            int frame = enemy_get_anim_frame(enemy_list[i]);
            if (enemy_get_anim_current(enemy_list[i]) == ANIM_DEATH) {
                if (frame < animations[clip].keyframeCount - 1) frame++;
            } else if (enemy_get_anim_current(enemy_list[i]) == ANIM_ATK) {
                if (frame < animations[clip].keyframeCount - 1) {
                    frame++;
                } else {
                    en_attacking[i] = false;
                }
            } else if (enemy_get_anim_current(enemy_list[i]) != ANIM_IDLE) {
                frame = (frame + 1) % animations[clip].keyframeCount;
            } else {
                frame = 0;
            }
            enemy_set_anim_frame(enemy_list[i], frame);
            UpdateModelAnimation(enemy_models[i], animations[clip], enemy_get_anim_frame(enemy_list[i]));
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
            CollisionResult_Capsule_t body_col = collision_resolve_capsules(
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
            if (enemy_get_anim_current(enemy_list[i]) == ANIM_ATK && anim_frame == ATK_IMPACT_FRAME) {
                CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                    player_collider_snapshot,
                    enemy_get_hitbox(enemy_list[i], pl_pos)
                );
                if (hit.penetration_depth > 0.0f) {
                    player_take_damage(pl, 10.0f);
                }
            }

            if (player_get_anim_current(pl) == ANIM_ATK && anim_frame == ATK_IMPACT_FRAME) {
                CollisionResult_Capsule_t hit = collision_resolve_capsule_box(
                    enemy_get_collider(enemy_list[i]), 
                    player_get_hitbox(pl)
                );
                if (hit.penetration_depth > 0.0f) {
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
        CapsuleCollider3D_t * enbox_list[ENEMY_NUM];
        float head_offset[ENEMY_NUM];
        Vector3 en_top_head[ENEMY_NUM];
        Vector2 en_screen_pos[ENEMY_NUM];
        int en_bar_x[ENEMY_NUM];
        int en_bar_y[ENEMY_NUM];
        int e1_bar_width = 100;

        for (int i = 0; i < ENEMY_NUM; ++i) {
            enbox_list[i] = enemy_get_collider(enemy_list[i]);
            head_offset[i] = geometry_capsule_get_coord(enbox_list[i], 1).y + geometry_capsule_get_radius(enbox_list[i]);
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
