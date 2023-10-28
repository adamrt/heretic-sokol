#include <float.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include "bin.h"
#include "gns.h"
#include "maths.h"
#include "mesh.h"

// forward declarations
static vec2 process_tex_coords(f32 u, f32 v, u8 page);
static vec3 mesh_center_transform(mesh_t *mesh);

b8 read_map(int map, mesh_t* mesh) {
    char *filename = "/home/adam/media/emu/fft.bin";
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("failed to open %s\n", filename);
        return  false;
    }

    int sector = gns_sectors[map];

    file_t gns = {0};
    if (!read_file(f, sector, GNS_MAX_SIZE, &gns)) {
        printf("failed to read gns\n");
        return false;
    }

    record_t records[RECORD_MAX_NUM] = {0};
    u16 num_records = {0};
    if (!read_records(&gns, records, &num_records)) {
        printf("failed to read records\n");
        return false;
    }

    for (int i = 0; i < num_records; i++) {
        record_t record = records[i];

        file_t resource = {0};
        if (!read_file(f, record.sector, record.len, &resource)) {
            printf("failed to read resource\n");
            return false;
        }

        switch (record.type) {
        case ResourceMeshPrimary:
            if (!read_mesh(&resource, mesh)) {
                printf("failed to read mesh\n");
                return false;
            }
            break;
        case ResourceTexture:
            if (!read_texture(&resource, mesh)) {
                printf("failed to read texture\n");
                return false;
            }
            break;
        case ResourceMeshOverride:
            // Sometimes there is no primary mesh (ie MAP002.GNS), there is
            // only an override. Usually a non-battle map. So we treat this
            // one as the primary, only if the primary hasn't been set. Kinda
            // Hacky until we start treating each GNS Record as a Scenario.
            if (!mesh->is_mesh_valid) {
                if (!read_mesh(&resource, mesh)) {
                    printf("failed to read alt mesh\n");
                    return false;
                }
            }
            break;
        default:
            break;
        }
    }

    return true;
}

b8 read_records(file_t* f, record_t *out_records, u16 *out_num_records) {
    while (true) {
        u16 header_unknown = read_u16(f);
        if (header_unknown != 0x22 && header_unknown != 0x30 && header_unknown != 0x70) {
            printf("invalid header 0x%02X\n", header_unknown);
            return false;
        }

        u8 arrangement = read_u8(f);
        u8 time_weather = read_u8(f);
        u16 file_type = read_u16(f);
        (void)read_u16(f); // padding
        u16 file_sector = read_u16(f);
        (void)read_u16(f); // padding
        u32 file_length = read_u32(f);
        (void)read_u32(f); // padding

        // This record type marks the end of the records for this GNS file.
        if (file_type == ResourceEnd) {
            return true;
        }

        out_records[*out_num_records] = (record_t) {
            .arrangement = arrangement,
            .time       = (time_weather >> 7) & 0x1,
            .weather    = (time_weather >> 4) & 0x7,
            .type       = file_type,
            .sector     = file_sector,
            .len        = file_length,
        };

        (*out_num_records)++;

        // Satefy check in case there is a bad read.
        if (*out_num_records >= RECORD_MAX_NUM) {
            printf("too many records\n");
            return false;
        }
    }
}

b8 read_mesh(file_t *f, mesh_t *mesh) {
    // 0x40 is always the location of the primary mesh pointer.
    // 0xC4 is always the primary mesh pointer.
    f->offset = 0x40;
    u32 primary_mesh_ptr = read_u32(f);
    assert(primary_mesh_ptr == 0xC4);
    f->offset = primary_mesh_ptr;

    // The number of each type of polygon.
    u16 N = read_u16(f); // Textured triangles
    u16 P = read_u16(f); // Textured quads
    u16 Q = read_u16(f); // Untextured triangles
    u16 R = read_u16(f); // Untextured quads

    // Validate maximum values
    if (N > 512 || P > 768 || Q > 64 || R > 256) {
        return false;
    }

    int index = 0;

    // Textured triangle vertices
    for (int i = index; i < N*3; i = i + 3) {
        mesh->vertices[i+0].position = read_position(f);
        mesh->vertices[i+1].position = read_position(f);
        mesh->vertices[i+2].position = read_position(f);
    }

    index = index + (N * 3);

    // Textured quad vertices. Split into 2 triangles.
    for (int i = index; i < index + (P*2*3); i = i + 6) {
        vec3 a = read_position(f);
        vec3 b = read_position(f);
        vec3 c = read_position(f);
        vec3 d = read_position(f);

        // Triangle A
        mesh->vertices[i+0].position = a;
        mesh->vertices[i+1].position = b;
        mesh->vertices[i+2].position = c;

        // Triangle B
        mesh->vertices[i+3].position = b;
        mesh->vertices[i+4].position = d;
        mesh->vertices[i+5].position = c;

    }

    index = index + (P*2*3);

    // Untextured triangle vertices
    for (int i = index; i < index + (Q*3); i = i + 3) {
        mesh->vertices[i+0].position = read_position(f);
        mesh->vertices[i+1].position = read_position(f);
        mesh->vertices[i+2].position = read_position(f);
    }

    index = index + (Q * 3);

    // Untextured quad vertices. Split into 2 triangles.
    for (int i = index; i < index + (R*2*3); i = i + 6) {
        vec3 a = read_position(f);
        vec3 b = read_position(f);
        vec3 c = read_position(f);
        vec3 d = read_position(f);

        // Triangle A
        mesh->vertices[i+0].position = a;
        mesh->vertices[i+1].position = b;
        mesh->vertices[i+2].position = c;

        // Triangle B
        mesh->vertices[i+3].position = b;
        mesh->vertices[i+4].position = d;
        mesh->vertices[i+5].position = c;
    }

    index = index + (R * 2 * 3);

    // Record number of vertices.
    // Should be equal to (N*3)+(P*3*2)+(Q*3)+(R*3*2)
    mesh->num_vertices = index;

    u32 expected_num_vertices = (u32)(N*3)+(P*3*2)+(Q*3)+(R*3*2);
    if (mesh->num_vertices != expected_num_vertices) {
        return false;
    }

    // Reset index so we can start over for normals, using the same vertices.
    index = 0;

    // Triangle normals
    for (int i = index; i < N*3; i = i + 3) {
        mesh->vertices[i+0].normal = read_normal(f);
        mesh->vertices[i+1].normal = read_normal(f);
        mesh->vertices[i+2].normal = read_normal(f);
    }

    index = index + (N * 3);

    // Quad normals. Split into 2 triangles.
    for (int i = index; i < index + (P*2*3); i = i + 6) {
        vec3 a = read_normal(f);
        vec3 b = read_normal(f);
        vec3 c = read_normal(f);
        vec3 d = read_normal(f);

        // Triangle A
        mesh->vertices[i+0].normal = a;
        mesh->vertices[i+1].normal = b;
        mesh->vertices[i+2].normal = c;

        // Triangle B
        mesh->vertices[i+3].normal = b;
        mesh->vertices[i+4].normal = d;
        mesh->vertices[i+5].normal = c;
    }

    // Reset index so we can start over for texture data, using the same vertices.
    index = 0;

    // Triangle UV
    for (int i = index; i < N*3; i = i + 3) {
        f32 au = read_u8(f);
        f32 av = read_u8(f);
        f32 palette = read_u8(f);
        (void)read_u8(f); // padding
        f32 bu = read_u8(f);
        f32 bv = read_u8(f);
        f32 page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f); // padding
        f32 cu = read_u8(f);
        f32 cv = read_u8(f);

        vec2 a = process_tex_coords(au, av, page);
        vec2 b = process_tex_coords(bu, bv, page);
        vec2 c = process_tex_coords(cu, cv, page);

        mesh->vertices[i+0].texcoords = a;
        mesh->vertices[i+0].palette = palette;
        mesh->vertices[i+1].texcoords = b;
        mesh->vertices[i+1].palette = palette;
        mesh->vertices[i+2].texcoords = c;
        mesh->vertices[i+2].palette = palette;
    }

    index = index + (N * 3);

    // Quad UV. Split into 2 triangles.
    for (int i = index; i < index + (P*2*3); i = i + 6) {
        f32 au = read_u8(f);
        f32 av = read_u8(f);
        f32 palette = read_u8(f);
        (void)read_u8(f); // padding
        f32 bu = read_u8(f);
        f32 bv = read_u8(f);
        f32 page = (read_u8(f) & 0x03); // 0b00000011
        (void)read_u8(f); // padding
        f32 cu = read_u8(f);
        f32 cv = read_u8(f);
        f32 du = read_u8(f);
        f32 dv = read_u8(f);

        vec2 a = process_tex_coords(au, av, page);
        vec2 b = process_tex_coords(bu, bv, page);
        vec2 c = process_tex_coords(cu, cv, page);
        vec2 d = process_tex_coords(du, dv, page);

        // Triangle A
        mesh->vertices[i+0].texcoords = a;
        mesh->vertices[i+0].palette = palette;
        mesh->vertices[i+1].texcoords = b;
        mesh->vertices[i+1].palette = palette;
        mesh->vertices[i+2].texcoords = c;
        mesh->vertices[i+2].palette = palette;

        // Triangle B
        mesh->vertices[i+3].texcoords = b;
        mesh->vertices[i+3].palette = palette;
        mesh->vertices[i+4].texcoords = d;
        mesh->vertices[i+4].palette = palette;
        mesh->vertices[i+5].texcoords = c;
        mesh->vertices[i+5].palette = palette;
    }

    read_palette(f, mesh);
    read_lights(f, mesh);
    read_background(f, mesh);

    mesh->center_transform = mesh_center_transform(mesh);

    mesh->is_mesh_valid = true;
    return true;
}


// 16 palettes of 16 colors of 4 bytes
b8 read_palette(file_t *f, mesh_t *mesh) {
    f->offset = 0x44;
    u32 intra_file_ptr = read_u32(f);
    f->offset = intra_file_ptr;

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4 c = read_rgb15(f);
        mesh->palette[i+0] = c.x;
        mesh->palette[i+1] = c.y;
        mesh->palette[i+2] = c.z;
        mesh->palette[i+3] = c.w;
    }

    return true;
}

b8 read_lights(file_t *f, mesh_t *mesh) {
    f->offset = 0x64;
    u32 intra_file_ptr = read_u32(f);
    f->offset = intra_file_ptr;

    mesh->dir_lights[0].color.x = read_f1x3x12(f);
    mesh->dir_lights[1].color.x = read_f1x3x12(f);
    mesh->dir_lights[2].color.x = read_f1x3x12(f);
    mesh->dir_lights[0].color.y = read_f1x3x12(f);
    mesh->dir_lights[1].color.y = read_f1x3x12(f);
    mesh->dir_lights[2].color.y = read_f1x3x12(f);
    mesh->dir_lights[0].color.z = read_f1x3x12(f);
    mesh->dir_lights[1].color.z = read_f1x3x12(f);
    mesh->dir_lights[2].color.z = read_f1x3x12(f);

    mesh->dir_lights[0].position = read_position(f);
    mesh->dir_lights[1].position = read_position(f);
    mesh->dir_lights[2].position = read_position(f);

    mesh->ambient_light_color = read_rgb8(f);

    return true;
}

b8 read_background(file_t *f, mesh_t *mesh) {
    mesh->background_top = read_rgb8(f);
    mesh->background_top = read_rgb8(f);
    return true;
}

b8 read_texture(file_t *f, mesh_t *mesh) {
    u8 raw_pixels[TEXTURE_RAW_SIZE];
    memcpy(&raw_pixels, f, TEXTURE_RAW_SIZE * sizeof(u8));

    for (int i = 0, j = 0; i < TEXTURE_RAW_SIZE; i++, j += 8) {
        u8 raw_pixel = raw_pixels[i];
        u8 right = ((raw_pixel & 0x0F));
        u8 left  = ((raw_pixel & 0xF0) >> 4);
        mesh->texture[j+0] = right;
        mesh->texture[j+1] = right;
        mesh->texture[j+2] = right;
        mesh->texture[j+3] = right;
        mesh->texture[j+4] = left;
        mesh->texture[j+5] = left;
        mesh->texture[j+6] = left;
        mesh->texture[j+7] = left;
    }

    // Scaling the RGB values from 0-16 to 0-255 so they are visible in ImGUI.
    for (int i = 0; i < TEXTURE_NUM_BYTES; i = i + 4) {
        mesh->texture_display[i+0] = mesh->texture[i+0] * 17;
        mesh->texture_display[i+1] = mesh->texture[i+1] * 17;
        mesh->texture_display[i+2] = mesh->texture[i+2] * 17;
        mesh->texture_display[i+3] = 255;
    }

    return true;
}

f32 read_f1x3x12(file_t *f) {
    f32 value = read_i16(f);
    return value / 4096.0f;
}

vec3 read_position(file_t *f) {
    f32 x = read_i16(f);
    f32 y = read_i16(f);
    f32 z = read_i16(f);

    x =  x / 100.0;
    y = -y / 100.0;
    z = -z / 100.0;

    return (vec3){ x, y, z };
}

vec3 read_normal(file_t *f) {
    f32 x = read_f1x3x12(f);
    f32 y = read_f1x3x12(f);
    f32 z = read_f1x3x12(f);

    y = -y;
    z = -z;

    return (vec3){ x, y, z };
}

vec4 read_rgb15(file_t *f) {
        u16 val = read_u16(f);
        u8 a = val == 0 ? 0x00 : 0xFF;
        u8 b = (val & 0x7C00) >> 7; // 0b0111110000000000
        u8 g = (val & 0x03E0) >> 2; // 0b0000001111100000
        u8 r = (val & 0x001F) << 3; // 0b0000000000011111
        return (vec4){r,g,b,a};
}

vec3 read_rgb8(file_t *f) {
    f32 r = (f32)read_u8(f) / 255.0f;
    f32 g = (f32)read_u8(f) / 255.0f;
    f32 b = (f32)read_u8(f) / 255.0f;
    return (vec3){ r, g, b };
}

// process_tex_coords has two functions:
//
// 1. Update the v coordinate to the specific page of the texture. FFT
//    Textures have 4 pages (256x1024) and the original V specifies
//    the pixel on one of the 4 pages. Multiply the page by the height
//    of a single page (256).
// 2. Normalize the coordinates that can be U:0-255 and V:0-1023. Just
//    divide them by their max to get a 0.0-1.0 value.
static vec2 process_tex_coords(f32 u, f32 v, u8 page) {
    u = u / 255.0f;
    v = (v + (page * 256)) / 1023.0f;
    return (vec2){ u, v };
}


static vec3 mesh_center_transform(mesh_t *mesh) {
    vec3 vmin = {.x=FLT_MAX, .y=FLT_MAX, .z=FLT_MAX};
    vec3 vmax = {.x=FLT_MIN, .y=FLT_MIN, .z=FLT_MIN};

    for (u32 i = 0; i < mesh->num_vertices; i++) {
        vec3 p = mesh->vertices[i].position;

        vmin.x = fmin(p.x, vmin.x);
        vmin.y = fmin(p.y, vmin.y);
        vmin.z = fmin(p.z, vmin.z);

        vmax.x = fmax(p.x, vmax.x);
        vmax.y = fmax(p.y, vmax.y);
        vmax.z = fmax(p.z, vmax.z);
    }

    return (vec3){
        .x = -(vmax.x + vmin.x) / 2.0,
        .y = -0.5, // maps already on 0.0. the -0.5 lowers it just a bit.
        .z = -(vmax.z + vmin.z) / 2.0,
    };
}
