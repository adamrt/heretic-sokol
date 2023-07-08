#include "camera.h"
#include "cube.h"
#include "heretic.h"
#include "mesh.h"

#define SOKOL_IMPL
#define SOKOL_GLCORE33
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#include "shaders/standard.glsl.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "hmmmath.h"

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
static void draw_ui(void);
static void next_map(void);
static void prev_map(void);
static void load_map(int map);

static struct {
    f32 time;
    bool rotate;
    f32  rotate_amt;
    int draw_mode;

    int mapnum;

    camera_t cam;
    mesh_t mesh;

    // display version of the texture (scaled rgb);
    sg_image maptex;
    sg_image mappalette;

    vec3_t ambient_color;

    sg_shader basic_shader;
    sg_shader light_shader;

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

    cam_init(&g.cam, &(camera_desc_t){
        .latitude = 30.0f,
        .longitude = 35.0f,
    });

    g.rotate = false;
    g.rotate_amt = 0.0f;

    g.draw_mode = 0;
    g.ambient_color = v3_new(1.0f, 1.0f, 1.0f);
    g.mapnum = 49;

    load_map(g.mapnum);

    // initial clear color
    g.pass_action = (sg_pass_action) {
        .colors[0] = { .load_action = SG_LOADACTION_CLEAR, .clear_value = { 0.2f, 0.3f, 0.3f, 1.0f } }
    };
}

static void event(const sapp_event* ev) {
    if (ev->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (ev->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_quit();
        }
        if (ev->key_code == SAPP_KEYCODE_K) {
            next_map();
        }
        if (ev->key_code == SAPP_KEYCODE_J) {
            prev_map();
        }

    }

    // Always register.
    poll_handle_event(ev);

    if (simgui_handle_event(ev)) {
        return;
    }

    // Only register if imgui doesn't handle it.
    cam_handle_event(&g.cam, ev);
}

static void frame(void) {
    simgui_new_frame(&(simgui_frame_desc_t){
        .width = sapp_width(),
        .height = sapp_height(),
        .delta_time = sapp_frame_duration(),
        .dpi_scale = sapp_dpi_scale(),
    });

    g.time += (f32)sapp_frame_duration();
    if (g.rotate) {
        g.rotate_amt += (f32)sapp_frame_duration() * 60.0;
    }

    cam_update(&g.cam, sapp_width(), sapp_height());

    draw_ui();

    sg_begin_default_pass(&g.pass_action, sapp_width(), sapp_height());

    // Basic object w/ texture
    {
        sg_apply_pipeline(g.pipe_object);
        sg_apply_bindings(&g.bind_object);

        // Vertex
        mat4_t model = m4_new(1.0f);
        model = m4_mul(model, m4_rotate(angle_deg(g.rotate_amt), v3_new(0.0f, 1.0f, 0.0f)));
        model = m4_mul(model, m4_translate(g.mesh.center_transform));
        vs_basic_params_t vs_params = {
            .u_projection = g.cam.proj,
            .u_view = g.cam.view,
            .u_model = model,
        };
        sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_basic_params, &SG_RANGE(vs_params));

        // Fragment
        fs_basic_params_t fs_params = {
            .u_draw_mode = g.draw_mode,
            .u_ambient_color = g.ambient_color,
        };
        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_basic_params, &SG_RANGE(fs_params));

        fs_dir_lights_t fs_lights = {0};
        for (int i = 0; i < 3; i++) {
            vec3_t c = g.mesh.dir_lights[i].color;
            vec3_t p = g.mesh.dir_lights[i].position;
            fs_lights.color[i] = (vec4_t){c.R, c.G, c.B, 255.0};
            fs_lights.position[i] = (vec4_t){p.X, p.Y, p.Z, 255.0};
        }
        sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_dir_lights, &SG_RANGE(fs_lights));

        sg_draw(0, g.mesh.num_vertices, 1);
    }

    // Light cube
    {
        sg_apply_pipeline(g.pipe_light);
        sg_apply_bindings(&g.bind_light);

        vs_light_params_t vs_params = {
            .projection = g.cam.proj,
            .view = g.cam.view,
        };

        // Vertex
        for (int i = 0; i < 3; i++) {
            mat4_t model = m4_new(1.0f);
            model = m4_mul(model, m4_translate(g.mesh.dir_lights[i].position));
            model = m4_mul(model, m4_scale(v3_new(0.2f, 0.2f, 0.2f)));
            vs_params.model = model;
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_light_params, &SG_RANGE(vs_params));

            fs_light_params_t fs_params = {.u_light_color = g.mesh.dir_lights[i].color,};
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_light_params, &SG_RANGE(fs_params));

            sg_draw(0, 36, 1);
        }

    }

    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void load_map(int map) {
    g.mesh = (mesh_t){0};

    if (!mesh_from_map(map, &g.mesh)) {
        printf("failed to open map file\n");
        exit(1);
    }

    sg_destroy_shader(g.basic_shader);
    sg_destroy_shader(g.light_shader);

    g.basic_shader = sg_make_shader(basic_shader_desc(sg_query_backend()));
    g.light_shader = sg_make_shader(light_shader_desc(sg_query_backend()));


    sg_destroy_pipeline(g.pipe_object);
    g.pipe_object = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = g.basic_shader,
        .face_winding = SG_FACEWINDING_CW,
        .cull_mode = SG_CULLMODE_BACK,
        .layout = {
            .attrs = {
                [ATTR_vs_basic_a_pos].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_basic_a_normal].format = SG_VERTEXFORMAT_FLOAT3,
                [ATTR_vs_basic_a_uv].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .label = "cube-pipeline"
    });

    sg_destroy_pipeline(g.pipe_light);
    g.pipe_light = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = g.light_shader,
        .layout = {
            .buffers[0].stride = 36,
            .attrs = {
                [ATTR_vs_light_aPos].format = SG_VERTEXFORMAT_FLOAT3,
            }
        },
        .depth = {.compare = SG_COMPAREFUNC_LESS_EQUAL, .write_enabled = true},
        .label = "light-pipeline"
    });


    sg_destroy_buffer(g.bind_light.vertex_buffers[0]);
    g.bind_light.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(cube_vertices),
        .label = "light-vertices"
    });


    sg_destroy_buffer(g.bind_object.vertex_buffers[0]);
    g.bind_object.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(g.mesh.vertices),
        .label = "map-vertices"
    });

    sg_destroy_image(g.bind_object.fs_images[SLOT_u_tex]);
    g.bind_object.fs_images[SLOT_u_tex] = sg_alloc_image();
    sg_init_image(g.bind_object.fs_images[SLOT_u_tex], &(sg_image_desc){
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .data.subimage[0][0] = {
            .ptr = g.mesh.texture,
            .size = (size_t)(TEXTURE_NUM_BYTES),
        },
        .label = "map-texture"
    });

    sg_destroy_image(g.maptex);
    g.maptex = sg_make_image(&(sg_image_desc){
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .data.subimage[0][0] = {
            .ptr = g.mesh.texture_display,
            .size = (size_t)(TEXTURE_NUM_BYTES),
        },
        .label = "map-texture-scaled"
    });

    sg_destroy_image(g.bind_object.fs_images[SLOT_u_palette]);
    g.bind_object.fs_images[SLOT_u_palette] = sg_alloc_image();
    sg_init_image(g.bind_object.fs_images[SLOT_u_palette], &(sg_image_desc){
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .width = 16 * 16,
        .height = 1,
        .data.subimage[0][0] = {
            .ptr = g.mesh.palette,
            .size = (size_t)(PALETTE_NUM_BYTES),
        },
        .label = "palette-texture"
    });

    sg_destroy_image(g.mappalette);
    g.mappalette = sg_make_image(&(sg_image_desc){
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .width = 16,
        .height = 16,
        .data.subimage[0][0] = {
            .ptr = g.mesh.palette,
            .size = (size_t)(PALETTE_NUM_BYTES),
        },
        .label = "palette-texture-squared"
    });
};

static void next_map(void) {
    g.mapnum++;
    if (g.mapnum > 119) {
        g.mapnum = 1;
    }
    load_map(g.mapnum);
}

static void prev_map(void) {
    g.mapnum--;
    if (g.mapnum < 1) {
        g.mapnum = 119;
    }
    load_map(g.mapnum);
}

static void draw_ui(void) {
    igSetNextWindowPos((ImVec2){sapp_width()-300, 10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){288, 731}, ImGuiCond_Once);
    igBegin("Graphics", 0, ImGuiWindowFlags_None);
    if (!igCollapsingHeader_TreeNodeFlags("Palette", 0)) {
        igImage((ImTextureID)(uintptr_t)g.mappalette.id, (ImVec2){256,256}, (ImVec2){0.0,0.0}, (ImVec2){1.0,1.0}, (ImVec4){1,1,1,1.0}, (ImVec4){1,1,1,1.0});
    }
    if (!igCollapsingHeader_TreeNodeFlags("Texture", 0)) {
        igImage((ImTextureID)(uintptr_t)g.maptex.id, (ImVec2){256,1024}, (ImVec2){0.0,0.0}, (ImVec2){1.0,1.0}, (ImVec4){1,1,1,1.0}, (ImVec4){1,1,1,1.0});
    }
    igEnd();

    igSetNextWindowPos((ImVec2){10,10}, ImGuiCond_Once, (ImVec2){0,0});
    igSetNextWindowSize((ImVec2){480, 550}, ImGuiCond_Once);
    igBegin("Heretic", 0, ImGuiWindowFlags_None);
    char map_title[10];
    sprintf(map_title, "Map %d", g.mapnum);
    igText(map_title);

    if (!igCollapsingHeader_TreeNodeFlags("Scene", 0)) {
        igRadioButton_IntPtr("Orthographic", &g.cam.proj_type, 1); igSameLine(100, 30);
        igRadioButton_IntPtr("Perspective", &g.cam.proj_type, 0);
        igRadioButton_IntPtr("Textured", &g.draw_mode, 0); igSameLine(100, 10);
        igRadioButton_IntPtr("Normals", &g.draw_mode, 1);igSameLine(200, 10);
        igRadioButton_IntPtr("Color", &g.draw_mode, 2);
        igColorEdit3("Background", &g.pass_action.colors[0].clear_value.r, ImGuiColorEditFlags_None);
        igText("");
    }
    if (!igCollapsingHeader_TreeNodeFlags("Camera", 0)) {
        igCheckbox("Rotate", &g.rotate);
        igSliderFloat("Latitude", &g.cam.latitude, -85.0f, 85.0f, "%0.2f", 0);
        igSliderFloat("Longitude", &g.cam.longitude, 0.0f, 360.0f, "%0.2f", 0);
        igText("");
    }

    if (!igCollapsingHeader_TreeNodeFlags("Lights", 0)) {
        igSeparatorText("Ambient");
        igColorEdit3("Color", &g.ambient_color, ImGuiColorEditFlags_None);
        for (int i = 0; i < 3; i++) {
            igPushID_Int(i);
            char title[10];
            sprintf(title, "Light %d", i);
            igSeparatorText(title);
            igSliderFloat3("Position", &g.mesh.dir_lights[i].position, -5000.0f, 5000.0f, "%0.2f", 0);
            igColorEdit3("Color", &g.mesh.dir_lights[i].color, ImGuiColorEditFlags_None);
            igPopID();
        }
        igText("");
    }
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
