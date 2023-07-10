#pragma once

#include <assert.h>
#include <math.h>
#include <string.h>

#include "sokol_app.h"

#include "defines.h"
#include "maths.h"

#define CAMERA_DEFAULT_FOV (45.0f)
#define CAMERA_DEFAULT_DIST (3.0f)

#define CAMERA_NEARZ (0.01f)
#define CAMERA_FARZ (100.0f)
#define CAMERA_MIN_DIST (2.5f)
#define CAMERA_MAX_DIST (100.0f)
#define CAMERA_MIN_LAT (-85.0f)
#define CAMERA_MAX_LAT (85.0f)

typedef struct {
    f32 distance;
    f32 fov;
    f32 latitude;
    f32 longitude;
} camera_desc_t;

enum {
  Perspective = 0,
  Orthographic = 1,
};

typedef struct {
    f32 distance;
    f32 latitude;
    f32 longitude;
    f32 fov;

    vec3 eye;
    vec3 target;

    u8 proj_type;

    mat4 view;
    mat4 proj;
} camera_t;

static f32 _cam_def(f32 val, f32 def) {
    return ((val == 0.0f) ? def:val);
}

/* initialize to default parameters */
void cam_init(camera_t* cam, const camera_desc_t* desc) {
    assert(cam && desc);
    memset(cam, 0, sizeof(camera_t));
    cam->distance = _cam_def(desc->distance, CAMERA_DEFAULT_DIST);
    cam->proj_type = Orthographic;

    cam->latitude= _cam_def(desc->latitude, 30.0f);
    cam->longitude= _cam_def(desc->longitude, 30.0f);
    cam->fov = _cam_def(desc->fov, CAMERA_DEFAULT_FOV);
}

/* feed mouse movement */
static void cam_orbit(camera_t* cam, f32 dx, f32 dy) {
    assert(cam);
    cam->longitude -= dx;
    if (cam->longitude < 0.0f) {
        cam->longitude += 360.0f;
    }
    if (cam->longitude > 360.0f) {
        cam->longitude -= 360.0f;
    }
    cam->latitude = clamp(cam->latitude + dy, CAMERA_MIN_LAT, CAMERA_MAX_LAT);
}

// feed zoom (mouse wheel) input
static void cam_zoom(camera_t* cam, f32 d) {
    assert(cam);
    cam->distance = clamp(CAMERA_MIN_DIST, cam->distance + d, CAMERA_MAX_DIST);
}

static vec3 _cam_euclidean(f32 latitude, f32 longitude) {
    const f32 lat = radians(latitude);
    const f32 lng = radians(longitude);
    return (vec3){cosf(lat) * sinf(lng), sinf(lat), cosf(lat) * cosf(lng)};
}

/* update the view, proj and view_proj matrix */
void cam_update(camera_t* cam, i32 fb_width, i32 fb_height) {
    assert(cam);
    assert((fb_width > 0) && (fb_height > 0));

    cam->eye = vec3_add(cam->target, vec3_mulf(_cam_euclidean(cam->latitude, cam->longitude), cam->distance));
    cam->view = mat4_look_at(cam->eye, cam->target, (vec3){0.0f, 1.0f, 0.0f});

    if (cam->proj_type == Perspective){
        const f32 w = (f32) fb_width;
        const f32 h = (f32) fb_height;
        cam->proj = mat4_perspective(cam->fov, w/h, CAMERA_NEARZ, CAMERA_FARZ);
    } else {
        const f32 aspect = (f32)fb_height/(f32)fb_width;
        const f32 w = 1.0 * cam->distance;
        const f32 h = 1.0 * aspect * cam->distance;
        cam->proj = mat4_orthographic(-w, w, -h, h, CAMERA_NEARZ, CAMERA_FARZ);
    }
}

/* handle sokol-app input events */
void cam_handle_event(camera_t* cam, const sapp_event* ev) {
    assert(cam);
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                sapp_lock_mouse(true);
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            if (ev->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
                sapp_lock_mouse(false);
            }
            break;
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            cam_zoom(cam, ev->scroll_y * -0.5f);
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            if (sapp_mouse_locked()) {
                cam_orbit(cam, ev->mouse_dx * 0.25f, ev->mouse_dy * 0.25f);
            }
            break;
        default:
            break;
    }
}
