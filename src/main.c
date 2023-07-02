// Sokol
#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"

// ImGUI
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "hmmmath.h"

#include "shaders/basic.glsl.h"

#include "heretic.h"
#include "cube.h"

// Key polling setup
#define BIT_INDEX(key) ((key) / 8)
#define BIT_MASK(key) (1 << ((key) % 8))
#define KEYDOWN_MAX 64
static uint8_t poll_keydown_state[KEYDOWN_MAX];

// Forward declarations;
static void init(void);
static void event(const sapp_event* ev);
static void frame(void);
static void cleanup(void);
static bool poll_keydown(sapp_keycode key);
static bool poll_handle_event(const sapp_event *evt);
static void camera_update(void);
static void camera_init(void);
static void camera_handle_event(const sapp_event* ev);
static void draw_ui(void);

static struct {
    f32 time;
    struct {
        vec3_t pos, front, up;
        f32 pitch, yaw, fov;
    } cam ;

    vec3_t object_color;
    vec3_t light_color;
    vec3_t light_pos;

    vs_params_t vs_shader;

    sg_pipeline pipe_object;
    sg_pipeline pipe_light;
    sg_bindings bind_object;
    sg_bindings bind_light;
    sg_pass_action pass_action;
} g;

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .event_cb = event,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Heretic",
        .width = 1280,
        .height = 960,
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}

static void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
        .logger.func = slog_func,
    });
    simgui_setup(&(simgui_desc_t){ 0 });

    camera_init();

    g.object_color = v3_new(1.0f, 0.5f, 0.31f);
    g.light_color = v3_new(1.0f, 1.0f, 1.0f);
    g.light_pos = v3_new(1.2f, 1.0f, 2.0f);

    g.bind_object.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(cube_vertices),
        .label = "cube-vertices"
    });

    g.bind_light.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(cube_vertices),
        .label = "light-vertices"
    });

    // create pipeline object
    g.pipe_object = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(basic_shader_desc(sg_query_backend())),
        .layout = {
            .attrs = {
                [ATTR_vs_basic_aPos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_basic_aTexCoord].format = SG_VERTEXFORMAT_FLOAT2,
            }
        },
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .label = "cube-pipeline"
    });

    stbi_set_flip_vertically_on_load(true);
    int desired_nch = 4;

    int wood_w, wood_h, wood_nch;
    unsigned char *wood = stbi_load("./res/wood.jpg", &wood_w, &wood_h, &wood_nch, desired_nch);
    if (wood == NULL) {
        printf("failed to open image\n");
        exit(1);
    };
    g.bind_object.fs_images[SLOT_texture1] = sg_alloc_image();
    sg_init_image(g.bind_object.fs_images[SLOT_texture1], &(sg_image_desc){
        .width = wood_w,
        .height = wood_h,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = wood,
            .size = (size_t)(wood_w * wood_h * 4),
        },
        .label = "wood-container-texture"
    });
    stbi_image_free(wood);

    g.pipe_light = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(light_shader_desc(sg_query_backend())),
        .layout = {
            .buffers[0].stride = 20,
            .attrs = {
                [ATTR_vs_light_aPos].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .label = "cube-pipeline"
    });

    // initial clear color
    g.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.2f, 0.3f, 0.3f, 1.0f } }
    };
}

static void event(const sapp_event* ev) {
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN && ev->key_code == SAPP_KEYCODE_ESCAPE) {
        sapp_quit();
    }

    if (simgui_handle_event(ev)) {
        return;
    };

    poll_handle_event(ev);
    camera_handle_event(ev);
}

// process_input will check keydown status to generate smooth movements.
// see the event callback for more information.
static void process_input(void) {
    const f32 delta = 5.0 * sapp_frame_duration();
    if (poll_keydown(SAPP_KEYCODE_W)) {
        g.cam.pos = v3_add(g.cam.pos, v3_mulf(g.cam.front, delta));
    }
    if (poll_keydown(SAPP_KEYCODE_S)) {
        g.cam.pos = v3_sub(g.cam.pos, v3_mulf(g.cam.front, delta));
    }
    if (poll_keydown(SAPP_KEYCODE_A)) {
        g.cam.pos = v3_sub(g.cam.pos, v3_mulf(v3_norm(v3_cross(g.cam.front, g.cam.up)), delta));
    }
    if (poll_keydown(SAPP_KEYCODE_D)) {
        g.cam.pos = v3_add(g.cam.pos, v3_mulf(v3_norm(v3_cross(g.cam.front, g.cam.up)), delta));
    }
}

static void frame(void) {
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    g.time += (f32)sapp_frame_duration();

    process_input();
    camera_update();
    draw_ui();

    sg_begin_default_pass(&g.pass_action, sapp_width(), sapp_height());

    // Basic object w/ texture
    {
        sg_apply_pipeline(g.pipe_object);
        sg_apply_bindings(&g.bind_object);

        // Vertex
        g.vs_shader.model = m4_new(1.0f);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(g.vs_shader));

        // Fragment
        fs_basic_params_t fs_params = {.lightColor = g.light_color,};
        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_basic_params, &SG_RANGE(fs_params));

        sg_draw(0, 36, 1);
    }

    // Light cube
    {
        sg_apply_pipeline(g.pipe_light);
        sg_apply_bindings(&g.bind_light);

        // Vertex
        // Overwrite shader model
        mat4_t model = m4_mul(m4_new(1.0f), m4_translate(g.light_pos));
        model = m4_mul(model, m4_scale(v3_new(0.2f, 0.2f, 0.2f)));
        g.vs_shader.model = model;
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(g.vs_shader));

        sg_draw(0, 36, 1);
    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void draw_ui(void) {
    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 150}, ImGuiCond_Once);
    igBegin("Heretic", 0, ImGuiWindowFlags_None);
    igColorEdit3("Background", &g.pass_action.colors[0].clear_value.r, ImGuiColorEditFlags_None);
    igSliderFloat("Pitch", &g.cam.pitch,-89.0, 89.0, "%0.2f", 0);
    igSliderFloat("Yaw", &g.cam.yaw,-360.0, 360.0, "%0.2f", 0);
    igEnd();
}

// poll_keydown returns a bool for a keys current pressed stated.
static bool poll_keydown(sapp_keycode key) {
    return poll_keydown_state[BIT_INDEX(key)] & BIT_MASK(key);
}

// poll_handle_event processes sokol/glfw events as they come in and
// sets keystate. This is used instead of the standard event callback
// because we want to check state per-frame, instead of only when a
// new event is fired. This is useful for smooth camera movement when
// holding a key down.
static bool poll_handle_event(const sapp_event* evt) {
    switch (evt->type) {
    case SAPP_EVENTTYPE_KEY_DOWN:
        poll_keydown_state[BIT_INDEX(evt->key_code)] |= BIT_MASK(evt->key_code);
        return true;
    case SAPP_EVENTTYPE_KEY_UP:
        poll_keydown_state[BIT_INDEX(evt->key_code)] &= ~BIT_MASK(evt->key_code);
        return true;
    case SAPP_EVENTTYPE_UNFOCUSED:
    case SAPP_EVENTTYPE_SUSPENDED:
    case SAPP_EVENTTYPE_ICONIFIED:
        memset(poll_keydown_state, 0, sizeof(poll_keydown_state));
        return true;
    default:
        return false;
    }
}

static void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
}

static void camera_update(void) {
    vec3_t direction = {
        .X = cos(angle_deg(g.cam.yaw)) * cos(angle_deg(g.cam.pitch)),
        .Y = sin(angle_deg(g.cam.pitch)),
        .Z = sin(angle_deg(g.cam.yaw)) * cos(angle_deg(g.cam.pitch)),
    };
    g.cam.front = v3_norm(direction);

    g.vs_shader.view = m4_lookat(g.cam.pos, v3_add(g.cam.pos, g.cam.front), g.cam.up);
    g.vs_shader.projection = m4_perspective(angle_deg(g.cam.fov), sapp_widthf() / sapp_heightf(), 0.1f, 100.0f);
}

static void camera_init(void) {
    g.cam.pos = v3_new(0.0f, 0.0f, 3.0f);
    g.cam.front = v3_new(0.0f, 0.0f, -1.0f);
    g.cam.up = v3_new(0.0f, 0.1f, 0.0f);
    g.cam.yaw = -90.0f;
    g.cam.pitch = 0.0;
    g.cam.fov = 45.0;
}

static void camera_handle_event(const sapp_event* ev) {
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

    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (sapp_mouse_locked()) {
            const f32 sensitivity = 0.2f;
            g.cam.pitch -= ev->mouse_dy * sensitivity;
            g.cam.yaw += ev->mouse_dx * sensitivity;
            if (g.cam.pitch > 89.0f) g.cam.pitch = 89.0f;
            if (g.cam.pitch < -89.0f) g.cam.pitch = -89.0f;
        }
        break;

    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        // Zooming
        g.cam.fov -= ev->scroll_y;
        if (g.cam.fov < 1.0f) g.cam.fov = 1.0f;
        if (g.cam.fov > 45.0f) g.cam.fov = 45.0f;
        break;

    default:
        break;
    }
}
