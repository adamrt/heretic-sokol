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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "hmmmath.h"

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
    // app
    f32 time;

    struct {
        HMM_Vec3 position;
        HMM_Vec3 front;
        HMM_Vec3 up;
    } camera ;

    vs_params_t vs_params;

    // sokol
    sg_pipeline pipe;
    sg_bindings bind;
    sg_pass_action pass_action;
} state;

typedef struct {
    f32 x, y, z;
    f32 u, v;
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

    state.vs_params.uSlider = 0.2f;
    state.camera.position = HMM_V3(0.0f, 0.0f, 3.0f);
    state.camera.front = HMM_V3(0.0f, 0.0f, -1.0f);
    state.camera.up = HMM_V3(0.0f, 0.1f, 0.0f);

    // vertex buffer object
    vertex vertices[] = {
        // positions           // texcoords
        {-0.5f, -0.5f, -0.5f,  0.0f, 0.0f},
        { 0.5f, -0.5f, -0.5f,  1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,  1.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f,  1.0f, 1.0f},
        {-0.5f,  0.5f, -0.5f,  0.0f, 1.0f},
        {-0.5f, -0.5f, -0.5f,  0.0f, 0.0f},

        {-0.5f, -0.5f,  0.5f,  0.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f,  1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f, 1.0f},
        {-0.5f,  0.5f,  0.5f,  0.0f, 1.0f},
        {-0.5f, -0.5f,  0.5f,  0.0f, 0.0f},

        {-0.5f,  0.5f,  0.5f,  1.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,  1.0f, 1.0f},
        {-0.5f, -0.5f, -0.5f,  0.0f, 1.0f},
        {-0.5f, -0.5f, -0.5f,  0.0f, 1.0f},
        {-0.5f, -0.5f,  0.5f,  0.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f,  1.0f, 0.0f},

        { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f},
        { 0.5f,  0.5f, -0.5f,  1.0f, 1.0f},
        { 0.5f, -0.5f, -0.5f,  0.0f, 1.0f},
        { 0.5f, -0.5f, -0.5f,  0.0f, 1.0f},
        { 0.5f, -0.5f,  0.5f,  0.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f},

        {-0.5f, -0.5f, -0.5f,  0.0f, 1.0f},
        { 0.5f, -0.5f, -0.5f,  1.0f, 1.0f},
        { 0.5f, -0.5f,  0.5f,  1.0f, 0.0f},
        { 0.5f, -0.5f,  0.5f,  1.0f, 0.0f},
        {-0.5f, -0.5f,  0.5f,  0.0f, 0.0f},
        {-0.5f, -0.5f, -0.5f,  0.0f, 1.0f},

        {-0.5f,  0.5f, -0.5f,  0.0f, 1.0f},
        { 0.5f,  0.5f, -0.5f,  1.0f, 1.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f},
        { 0.5f,  0.5f,  0.5f,  1.0f, 0.0f},
        {-0.5f,  0.5f,  0.5f,  0.0f, 0.0f},
        {-0.5f,  0.5f, -0.5f,  0.0f, 1.0f},
    };

    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(vertices),
        .label = "cube-vertices"
    });

    /* // element buffer object */
    /* u16 indices[] = { */
    /*     0, 1, 3,   // first triangle */
    /*     1, 2, 3    // second triangle */
    /* }; */
    /* state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){ */
    /*     .type = SG_BUFFERTYPE_INDEXBUFFER, */
    /*     .data = SG_RANGE(indices), */
    /*     .label = "cube-indices" */
    /* }); */

    // create shader
    sg_shader shd = sg_make_shader(basic_shader_desc(sg_query_backend()));

    // create pipeline object
    state.pipe = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = shd,
        // .index_type = SG_INDEXTYPE_UINT16,
        .layout = {
            .attrs = {
                [ATTR_vs_aPos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_aTexCoord].format = SG_VERTEXFORMAT_FLOAT2,
            }
        },
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true
        },
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
    state.bind.fs_images[SLOT_texture1] = sg_alloc_image();
    sg_init_image(state.bind.fs_images[SLOT_texture1], &(sg_image_desc){
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

    int face_w, face_h, face_nch;
    unsigned char *face = stbi_load("./res/face.png", &face_w, &face_h, &face_nch, desired_nch);
    if (face == NULL) {
        printf("failed to open image\n");
        exit(1);
    };
    state.bind.fs_images[SLOT_texture2] = sg_alloc_image();
    sg_init_image(state.bind.fs_images[SLOT_texture2], &(sg_image_desc){
        .width = face_w,
        .height = face_h,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .data.subimage[0][0] = {
            .ptr = face,
            .size = (size_t)(face_w * face_h * 4),
        },
        .label = "face-container-texture"
    });
    stbi_image_free(face);

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

        const float delta = 5.0 * sapp_frame_duration();

        if (ev->key_code == SAPP_KEYCODE_W) {
            state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(state.camera.front, delta));
        }
        if (ev->key_code == SAPP_KEYCODE_S) {
            state.camera.position = HMM_SubV3(state.camera.position, HMM_MulV3F(state.camera.front, delta));
        }
        if (ev->key_code == SAPP_KEYCODE_A) {
            state.camera.position = HMM_SubV3(state.camera.position, HMM_MulV3F(HMM_NormV3(HMM_Cross(state.camera.front, state.camera.up)), delta));
        }
        if (ev->key_code == SAPP_KEYCODE_D) {
            state.camera.position = HMM_AddV3(state.camera.position, HMM_MulV3F(HMM_NormV3(HMM_Cross(state.camera.front, state.camera.up)), delta));
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

    state.time += (f32)sapp_frame_duration();

    HMM_Vec3 cubePositions[] = {
        HMM_V3( 0.0f,  0.0f,  0.0f),
        HMM_V3( 2.0f,  5.0f, -15.0f),
        HMM_V3(-1.5f, -2.2f, -2.5f),
        HMM_V3(-3.8f, -2.0f, -12.3f),
        HMM_V3( 2.4f, -0.4f, -3.5f),
        HMM_V3(-1.7f,  3.0f, -7.5f),
        HMM_V3( 1.3f, -2.0f, -2.5f),
        HMM_V3( 1.5f,  2.0f, -2.5f),
        HMM_V3( 1.5f,  0.2f, -1.5f),
        HMM_V3(-1.3f,  1.0f, -1.5f)
    };

    state.vs_params.view = HMM_LookAt_RH(state.camera.position, HMM_AddV3(state.camera.position, state.camera.front), state.camera.up);
    state.vs_params.projection = HMM_Perspective_RH_NO(HMM_AngleDeg(45.0f), sapp_widthf() / sapp_heightf(), 0.1f, 100.0f);

    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){400, 100}, ImGuiCond_Once);
    igBegin("Dear ImGui!", 0, ImGuiWindowFlags_None);
    igColorEdit3("Background", &state.pass_action.colors[0].clear_value.r, ImGuiColorEditFlags_None);
    igSliderFloat("Slider", &state.vs_params.uSlider,0.0, 1.0, "%0.2f", 0);
    igEnd();

    sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
    sg_apply_pipeline(state.pipe);
    sg_apply_bindings(&state.bind);

    for(unsigned int i = 0; i < 10; i++) {
        state.vs_params.model = HMM_MulM4(HMM_M4D(1.0f), HMM_Translate(cubePositions[i]));
        state.vs_params.model = HMM_MulM4(state.vs_params.model, HMM_Rotate_RH(HMM_AngleDeg(20.0f*i), HMM_V3(1.0f, 0.3f, 0.5f)));
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &SG_RANGE(state.vs_params));
        sg_draw(0, 36, 1);
    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) {
    simgui_shutdown();
    sg_shutdown();
}
