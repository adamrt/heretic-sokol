// This file is for recording and checking keystate.
//
// This is used instead of the GLFW event callback because we want to
// check state per-frame, instead of only when a new event is fired.
// This is useful for smooth camera movement when holding a key down.
#pragma once

#include "sokol_app.h"

#include "defines.h"

#define BIT_INDEX(key) ((key) / 8)
#define BIT_MASK(key) (1 << ((key) % 8))
#define KEYDOWN_MAX 64

void keystate_handle_event(const sapp_event *evt);
b8   keystate_is_down(sapp_keycode key);
