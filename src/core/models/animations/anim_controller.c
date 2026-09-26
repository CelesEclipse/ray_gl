#include "anim_controller.h"

/* Return clip index to feed into animations array */
int anim_controller_resolve_clip(AnimState_t current_anim, AnimClipSet_t clips)
{
    int clip = 0;
    switch (current_anim) {
        case ANIM_WALK:     clip = clips.walk;     break;
        case ANIM_RUN:      clip = clips.run;      break;
        case ANIM_DEATH:    clip = clips.death;    break;
        case ANIM_ATK:      clip = clips.atk;      break;
        case ANIM_ATK_360:  clip = clips.atk360;   break;
        case ANIM_IDLE:
        default:            clip = clips.idle;     break;
    }
    return clip;
}

int anim_controller_advance(AnimState_t current_state, int current_frame, int keyframe, bool *out_finished)
{
    if (out_finished) *out_finished = false;
    if (keyframe <= 0) return 0;

    switch (current_state) {
        case ANIM_DEATH:
            if (current_frame < keyframe - 1) return current_frame + 1;
            return current_frame;

        case ANIM_ATK:
        case ANIM_ATK_360:
            if (current_frame < keyframe - 1) return current_frame + 1;
            if (out_finished) *out_finished = true; // swing completed this tick
            return current_frame;

        case ANIM_IDLE:
            return 0;

        case ANIM_WALK:
        case ANIM_RUN:
        default:
            return (current_frame + 1) % keyframe;
    }
}
