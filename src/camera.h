#pragma once

#include <assert.h>
#include <string.h>
#include <math.h>

#include "sokol_app.h"

#include "defines.h"
#include "hmmmath.h"

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

    vec3_t eye;
    vec3_t target;

    u8 proj_type;

    mat4_t view;
    mat4_t proj;
} camera_t;

static f32 _cam_def(f32 val, f32 def) {
    return ((val == 0.0f) ? def:val);
}

/* initialize to default parameters */
static void cam_init(camera_t* cam, const camera_desc_t* desc) {
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
    cam->latitude = HMM_Clamp(CAMERA_MIN_LAT, cam->latitude + dy, CAMERA_MAX_LAT);
}

// feed zoom (mouse wheel) input
static void cam_zoom(camera_t* cam, f32 d) {
    assert(cam);
    cam->distance = HMM_Clamp(CAMERA_MIN_DIST, cam->distance + d, CAMERA_MAX_DIST);
}

static vec3_t _cam_euclidean(f32 latitude, f32 longitude) {
    const f32 lat = angle_deg(latitude);
    const f32 lng = angle_deg(longitude);
    return v3_new(cosf(lat) * sinf(lng), sinf(lat), cosf(lat) * cosf(lng));
}

/* update the view, proj and view_proj matrix */
static void cam_update(camera_t* cam, i32 fb_width, i32 fb_height) {
    assert(cam);
    assert((fb_width > 0) && (fb_height > 0));

    cam->eye = v3_add(cam->target, v3_mulf(_cam_euclidean(cam->latitude, cam->longitude), cam->distance));
    cam->view = m4_lookat(cam->eye, cam->target, v3_new(0.0f, 1.0f, 0.0f));

    if (cam->proj_type == Perspective){
        const f32 w = (float) fb_width;
        const f32 h = (float) fb_height;
        cam->proj = m4_perspective(cam->fov, w/h, CAMERA_NEARZ, CAMERA_FARZ);
    } else {
        const f32 aspect = (float)fb_height/(float)fb_width;
        const f32 w = 1.0 * cam->distance;
        const f32 h = 1.0 * aspect * cam->distance;
        cam->proj = HMM_Orthographic_RH_NO(-w, w, -h, h, CAMERA_NEARZ, CAMERA_FARZ);
    }
}

/* handle sokol-app input events */
static void cam_handle_event(camera_t* cam, const sapp_event* ev) {
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
