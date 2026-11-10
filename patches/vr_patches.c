#include "patches.h"
#include "graphics.h"
#include "vr_patches.h"

static s32 vr_active = false;
static s32 vr_letterbox_hidden = false;
static View* vr_world_view = NULL;

void vr_play_update(PlayState* play) {
    vr_active = recomp_vr_active();
    vr_letterbox_hidden = vr_active && recomp_vr_hide_letterbox();
}

s32 vr_is_active(void) {
    return vr_active;
}

s32 vr_hide_letterbox(void) {
    return vr_letterbox_hidden;
}

void vr_set_world_view(View* view) {
    vr_world_view = view;
}

s32 vr_is_world_view(View* view) {
    return (view != NULL) && (view == vr_world_view);
}
