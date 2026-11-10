#ifndef __VR_PATCHES_H__
#define __VR_PATCHES_H__

#include "patches.h"

// Updates the cached VR state. Must be called once per frame before the game updates.
void vr_play_update(PlayState* play);

// Whether the game is currently being rendered to a VR headset. Cached once per frame.
s32 vr_is_active(void);

// Whether the letterbox bars that are shown during cutscenes should be hidden while in VR.
s32 vr_hide_letterbox(void);

// Sets the view that corresponds to the main world camera. Only valid while the play state is being drawn.
void vr_set_world_view(View* view);

// Whether the view is the main world camera, which receives head tracking in VR.
s32 vr_is_world_view(View* view);

#endif
