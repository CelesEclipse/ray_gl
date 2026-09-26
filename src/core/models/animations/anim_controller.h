#pragma once

#include <raylib.h>
#include "../../../common/common.h"

typedef struct
{
    int idle, walk, run, death, atk, atk360;
} AnimClipSet_t;

int anim_controller_resolve_clip(AnimState_t current_state, AnimClipSet_t clips);
int anim_controller_advance(AnimState_t current_state, int current_frame, int keyframe, bool *out_finished);
