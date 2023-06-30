#include <stdint.h>

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

#include "shaders/basic.glsl.h"

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef float  f32;
typedef double f64;

// Forward declarations;
static void init(void);
static void event(const sapp_event* ev);
static void frame(void);
static void cleanup(void);

static struct {
    sg_pipeline pipe;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

typedef struct {
    f32 x, y, z;
} vertex;

sapp_desc sokol_main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .event_cb = event,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .window_title = "Heretic",
        .width = 800,
        .height = 600,
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

    // vertex buffer object
    const vertex vertices[] = {
        { 0.5f,  0.5f, 0.0f},  // top right
        { 0.5f, -0.5f, 0.0f},  // bottom right
        {-0.5f, -0.5f, 0.0f},  // bottom left
        {-0.5f,  0.5f, 0.0f},  // top left
    };
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "square-vertices"
    });

    // element buffer object
    u16 indices[] = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .data = SG_RANGE(indices),
        .label = "square-indices"
    });

    // create shader
    sg_shader shd = sg_make_shader(basic_shader_desc(sg_query_backend()));

    // create pipeline object
    state.pipe = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [ATTR_vs_basic_position].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .label = "square-pipeline"
    });

    // initial clear color
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.2f, 0.3f, 0.3f, 1.0f } }
    };
}

static void event(const sapp_event* ev) {
    simgui_handle_event(ev);
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
    }
}

static void frame(void) {
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 100}, ImGuiCond_Once);
    igBegin("Dear ImGui!", 0, ImGuiWindowFlags_None);
    igColorEdit3("Background", &state.pass_action.colors[0].clear_value.r, ImGuiColorEditFlags_None);
    igEnd();

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pipe);
    sg_apply_bindings(&state.bind);
    sg_draw(0, 6, 1);

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
}
