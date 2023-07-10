#pragma once

#include <string.h>

#include "bin.h"
#include "defines.h"
#include "maths.h"

#define GNS_MAX_SIZE   2388
#define RECORD_MAX_NUM 100

#define MAX_VERTS 5000

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 1024
#define TEXTURE_NUM_PIXELS 262144 // 256 * 1024
#define TEXTURE_NUM_BYTES (262144 * 4) // 256 * 1024 * 4
#define TEXTURE_RAW_SIZE (TEXTURE_NUM_PIXELS / 2)

#define PALETTE_NUM_BYTES (16 * 16 * 4)

enum Resource {
    ResourceTexture      = 0x1701,
    ResourceMeshPrimary  = 0x2E01,
    ResourceMeshOverride = 0x2F01,
    ResourceMeshAlt      = 0x3001,
    ResourceEnd          = 0x3101,
};

enum Time {
  TimeDay,
  TimeNight,
};

// record_t represents a GNS record.
typedef struct {
    u16 sector;
    u64 len;
    u16 type;
    u8  arrangement;
    u8  time;
    u8  weather;
} record_t;

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 texcoords;
    f32  palette;
} vertex_t;

typedef struct {
    vec3 position;
    vec3 color;
} light_t;

typedef struct {
    vertex_t vertices[MAX_VERTS];
    u32 num_vertices;

    u8 texture[TEXTURE_NUM_BYTES];
    u8 texture_display[TEXTURE_NUM_BYTES];

    u8 palette[PALETTE_NUM_BYTES];

    light_t dir_lights[3];
    vec3 ambient_light;
    vec3 background_top;
    vec3 background_bottom;

    // Transform to center all vertices.
    vec3 center_transform;

    b8 is_mesh_valid;
    b8 is_texture_valid;
} mesh_t;

b8 read_map(int mapnum, mesh_t *out_mesh);
b8 read_records(file_t *f, record_t *out_records, u16 *out_num_records);
b8 read_mesh(file_t *f, mesh_t *out_mesh);
b8 read_texture(file_t *f, mesh_t *out_mesh);
b8 read_palette(file_t *f, mesh_t *out_mesh);
b8 read_lights(file_t *f, mesh_t *out_mesh);

f32  read_f1x3x12(file_t *f);
vec3 read_position(file_t *f);
vec3 read_normal(file_t *f);
vec4 read_rgb15(file_t *f);
