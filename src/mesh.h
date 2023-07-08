#pragma once

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "heretic.h"
#include "hmmmath.h"

#define MAX_VERTS 5000

#define MAP_SECTOR 33032
#define SECTOR_SIZE 2048
#define SECTOR_RAW_SIZE 2352
#define SECTOR_HEADER_SIZE 24

#define SECTOR_LEN 2048
#define MAX_GNS_RECORDS 100

#define TEXTURE_WIDTH 256
#define TEXTURE_HEIGHT 1024
#define TEXTURE_NUM_PIXELS 262144 // 256 * 1024
#define TEXTURE_NUM_BYTES (262144 * 4) // 256 * 1024 * 4
#define TEXTURE_RAW_SIZE (TEXTURE_NUM_PIXELS / 2)

#define PALETTE_NUM_BYTES (16 * 16 * 4)

enum FFTRecord {
    FFTRecordTexture = 0x1701,
    FFTRecordMeshPrimary = 0x2E01,
    FFTRecordMeshOverride = 0x2F01,
    FFTRecordMeshAlt = 0x3001,
    FFTRecordEnd = 0x3101,
};

typedef struct {
    u16 file_type;
    u16 file_sector;
} gns_record_t;

typedef struct {
    gns_record_t records[MAX_GNS_RECORDS];
    i8 num_records;
    bool is_valid;
} gns_t;

typedef struct {
    vec3_t position;
    vec3_t color;
} light_t;

typedef struct {
    vertex_t vertices[MAX_VERTS];
    u32 num_vertices;

    u8 texture[TEXTURE_NUM_BYTES];
    u8 texture_display[TEXTURE_NUM_BYTES];

    u8 palette[PALETTE_NUM_BYTES];

    light_t dir_lights[3];
    vec3_t ambient_light;
    vec3_t background_top;
    vec3_t background_bottom;

    // Transform to center all vertices.
    vec3_t center_transform;

    bool is_mesh_valid;
    bool is_texture_valid;
} mesh_t;

bool mesh_from_obj(mesh_t *mesh, char *filename);
bool mesh_from_map(int map, mesh_t *mesh);

void read_gns(FILE* f, int sector, gns_t *gns);
void read_mesh(FILE *f, int sector, mesh_t *mesh);
void read_texture(FILE *f, int sector, mesh_t *mesh);
void read_palette(FILE *f, int sector, mesh_t *mesh);
void read_lights(FILE *f, int sector, mesh_t *mesh);
vec4_t read_rgb15(FILE *f);

vec3_t mesh_center_transform(mesh_t *mesh);

vec3_t read_position(FILE *f);
vec3_t read_normal(FILE *f);

u8  read_u8(FILE *f);
u16 read_u16(FILE *f);
u32 read_u32(FILE *f);
i8  read_i8(FILE *f);
i16 read_i16(FILE *f);
i32 read_i32(FILE *f);
float read_f1x3x12(FILE *f);
