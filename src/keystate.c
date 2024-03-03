#include <string.h>

#include "keystate.h"

// The state of all keys currently down.
static u8 keystate[KEYDOWN_MAX];

// keystate_is_down returns a bool if that key is down.
b8 keystate_is_down(sapp_keycode key)
{
    return keystate[BIT_INDEX(key)] & BIT_MASK(key);
}

// keystate_handle_event records a current keys state for a GLFW event.
void keystate_handle_event(const sapp_event* ev)
{
    switch (ev->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        keystate[BIT_INDEX(ev->key_code)] |= BIT_MASK(ev->key_code);
        break;
    case SAPP_EVENTTYPE_KEY_UP:
        keystate[BIT_INDEX(ev->key_code)] &= ~BIT_MASK(ev->key_code);
        break;
    case SAPP_EVENTTYPE_UNFOCUSED:
    case SAPP_EVENTTYPE_SUSPENDED:
    case SAPP_EVENTTYPE_ICONIFIED:
        memset(keystate, 0, sizeof(keystate));
        break;
    default:
        break;
    }
}
