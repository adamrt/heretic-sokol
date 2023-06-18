#include <stdint.h>
#define HANDMADE_MATH_IMPLEMENTATION
#define HANDMADE_MATH_NO_SSE
#include "HandmadeMath.h"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
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
    float rx, ry;
    bool rotate;
    sg_pipeline pipe;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

typedef struct {
    f32 x, y, z;
    f32 r, g, b, a;
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

    /* a vertex buffer */
    const vertex vertices[] = {
        // positions            // colors
        { -0.8, -0.8, -0.8,   1.0, 0.0, 0.0, 1.0 },
        {  0.8, -0.8, -0.8,   1.0, 0.0, 0.0, 1.0 },
        {  0.8,  0.8, -0.8,   1.0, 0.0, 0.0, 1.0 },
        { -0.8,  0.8, -0.8,   1.0, 0.0, 0.0, 1.0 },

        { -0.8, -0.8,  0.8,   0.0, 1.0, 0.0, 1.0 },
        {  0.8, -0.8,  0.8,   0.0, 1.0, 0.0, 1.0 },
        {  0.8,  0.8,  0.8,   0.0, 1.0, 0.0, 1.0 },
        { -0.8,  0.8,  0.8,   0.0, 1.0, 0.0, 1.0 },

        { -0.8, -0.8, -0.8,   0.0, 0.0, 1.0, 1.0 },
        { -0.8,  0.8, -0.8,   0.0, 0.0, 1.0, 1.0 },
        { -0.8,  0.8,  0.8,   0.0, 0.0, 1.0, 1.0 },
        { -0.8, -0.8,  0.8,   0.0, 0.0, 1.0, 1.0 },

        { 0.8, -0.8, -0.8,    1.0, 1.0, 0.0, 1.0 },
        { 0.8,  0.8, -0.8,    1.0, 1.0, 0.0, 1.0 },
        { 0.8,  0.8,  0.8,    1.0, 1.0, 0.0, 1.0 },
        { 0.8, -0.8,  0.8,    1.0, 1.0, 0.0, 1.0 },

        { -0.8, -0.8, -0.8,   0.0, 1.0, 1.0, 1.0 },
        { -0.8, -0.8,  0.8,   0.0, 1.0, 1.0, 1.0 },
        {  0.8, -0.8,  0.8,   0.0, 1.0, 1.0, 1.0 },
        {  0.8, -0.8, -0.8,   0.0, 1.0, 1.0, 1.0 },

        { -0.8,  0.8, -0.8,   1.0, 0.0, 1.0, 1.0 },
        { -0.8,  0.8,  0.8,   1.0, 0.0, 1.0, 1.0 },
        {  0.8,  0.8,  0.8,   1.0, 0.0, 1.0, 1.0 },
        {  0.8,  0.8, -0.8,   1.0, 0.0, 1.0, 1.0 }
    };

    sg_buffer vbuf = sg_make_buffer(&(sg_buffer_desc){
            .data = SG_RANGE(vertices),
            .label = "cube-vertices"
    });

    /* create an index buffer for the cube */
    uint16_t indices[] = {
        0,   1,  2,   0,  2,  3,
        6,   5,  4,   7,  6,  4,
        8,   9, 10,   8, 10, 11,
        14, 13, 12,  15, 14, 12,
        16, 17, 18,  16, 18, 19,
        22, 21, 20,  23, 22, 20
    };
    sg_buffer ibuf = sg_make_buffer(&(sg_buffer_desc){
            .type = SG_BUFFERTYPE_INDEXBUFFER,
            .data = SG_RANGE(indices),
            .label = "cube-indices"
    });

    /* create shader */
    sg_shader shd = sg_make_shader(cube_shader_desc(sg_query_backend()));

    /* create pipeline object */
    state.pipe = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        .layout = {
            .attrs = {
                [ATTR_vs_position].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_color0].format   = SG_VERTEXFORMAT_FLOAT4
            }
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .depth = {
            .write_enabled = true,
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
        },
        .label = "triangle-pipeline"
    });



    state.bind = (sg_bindings) {
        .vertex_buffers[0] = vbuf,
        .index_buffer = ibuf
    };

    state.rotate = false;
    // initial clear color
    state.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.0f, 0.0f, 0.0f, 1.0 } }
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

    vs_params_t vs_params;
    const float w = sapp_widthf();
    const float h = sapp_heightf();
    const float t = (float)(sapp_frame_duration() * 60.0);
    hmm_mat4 proj = HMM_Perspective(60.0f, w/h, 0.01f, 10.0f);
    hmm_mat4 view = HMM_LookAt(HMM_Vec3(0.0f, 1.5f, 6.0f), HMM_Vec3(0.0f, 0.0f, 0.0f), HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 view_proj = HMM_MultiplyMat4(proj, view);
    if (state.rotate) {
        state.rx += 1.0f * t; state.ry += 2.0f * t;
    }
    hmm_mat4 rxm = HMM_Rotate(state.rx, HMM_Vec3(1.0f, 0.0f, 0.0f));
    hmm_mat4 rym = HMM_Rotate(state.ry, HMM_Vec3(0.0f, 1.0f, 0.0f));
    hmm_mat4 model = HMM_MultiplyMat4(rxm, rym);
    vs_params.mvp = HMM_MultiplyMat4(view_proj, model);

    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 100}, ImGuiCond_Once);
    igBegin("Dear ImGui!", 0, ImGuiWindowFlags_None);
    igColorEdit3("Background", &state.pass_action.colors[0].clear_value.r, ImGuiColorEditFlags_NoInputs);
    igCheckbox("Rotate", &state.rotate);
    igEnd();

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pipe);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 36, 1);

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
}
