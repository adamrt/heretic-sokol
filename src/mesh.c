#include <float.h>
#include <math.h>

#include "gns.h"
#include "maths.h"
#include "mesh.h"


b8 mesh_from_obj(mesh_t* mesh, char* filename)
{
    FILE* file;
    file = fopen(filename, "r");
    if (file == NULL) {
        perror("error");
        return false;
    }
    char line[1024];

    vec3 temp_vertices[MAX_VERTS];
    vec3 temp_normals[MAX_VERTS];
    vec2 temp_uvs[MAX_VERTS];
    u32 temp_num_vertices = {0};
    u32 temp_num_normals = {0};
    u32 temp_num_uvs = {0};

    while (fgets(line, 1024, file)) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            vec3 v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            temp_vertices[temp_num_vertices++] = v;
        }
        // normal information
        if (strncmp(line, "vn ", 3) == 0) {
            vec3 n;
            sscanf(line, "vn %f %f %f", &n.x, &n.y, &n.z);
            temp_normals[temp_num_normals++] = n;
        }
        // texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            vec2 uv = {0}; // extra space for fft
            sscanf(line, "vt %f %f", &uv.x, &uv.y);
            temp_uvs[temp_num_uvs++] = uv;
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            i32 vertex_indices[3];
            i32 uv_indices[3];
            i32 normal_indices[3];
            i32 matches = sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &uv_indices[0], &normal_indices[0],
                &vertex_indices[1], &uv_indices[1], &normal_indices[1],
                &vertex_indices[2], &uv_indices[2], &normal_indices[2]);
            if (matches != 9) {
                printf("File can't be parsed.\n");
                return false;
            }

            for (i32 i = 0; i < 3; i++) {
                vec3 v = temp_vertices[vertex_indices[i]-1];
                vec3 n = temp_normals[normal_indices[i]-1];
                vec2 t = temp_uvs[uv_indices[i]-1];
                mesh->vertices[mesh->num_vertices] = (vertex_t){v, n, t, 0.0f};;
                mesh->num_vertices++;

                if (mesh->num_vertices >= MAX_VERTS) {
                    printf("too many verts\n");
                    return false;
                }
            }
        }
    }
    fclose(file);
    return true;
}

b8 mesh_from_map(int map, mesh_t* mesh) {
    char *filename = "/home/adam/media/emu/fft.iso";
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Error opening %s\n", filename);
        return  false;
    }

    int sector = gns_sectors[map];

    gns_t gns = {0};
    read_gns(f, sector, &gns);
    if (!gns.is_valid) {
        printf("Error reading gns\n");
        return  false;
    }

    for (int i = 0; i < gns.num_records; i++) {
        gns_record_t record = gns.records[i];

        switch (record.file_type) {
        case FFTRecordMeshPrimary:
            read_mesh(f, record.file_sector, mesh);
            break; // from switch
        case FFTRecordTexture:
            read_texture(f, record.file_sector, mesh);
            break; // from switch
        case FFTRecordMeshOverride:
            // Sometimes there is no primary mesh (ie MAP002.GNS), there is
            // only an override. Usually a non-battle map. So we treat this
            // one as the primary, only if the primary hasn't been set. Kinda
            // Hacky until we start treating each GNS Record as a Scenario.
            if (!mesh->is_mesh_valid) {
                read_mesh(f, record.file_sector, mesh);
            }
            break; // from switch
        default:
            break; // from switch
        }
    }

    if (!mesh->is_mesh_valid || !mesh->is_texture_valid) {
        printf("the texture or mesh is invalid\n");
        return false;
    }

    return true;
}

/* static void seek_sector(FILE *f, int sector) { */
/*     i32 to = (sector * SECTOR_RAW_SIZE) + SECTOR_HEADER_SIZE; */
/*     fseek(f, to, SEEK_SET); */
/* } */

/* static void read_sector(FILE *f, int sector, u8 *bytes) { */
/*     seek_sector(f, sector); */
/*     size_t n = fread(bytes, sizeof(u8), SECTOR_SIZE, f); */
/*     if (n != SECTOR_SIZE) { */
/*         printf("failed to read sector"); */
/*         exit(1); */
/*     } */
/* } */

typedef struct {
    u8* data;
    size_t len;
} file_t;

/* static file_t read_file(FILE *f, int sector, size_t size)  { */
/*     u8* file_data = malloc(size * sizeof(u8)); // array to hold the result */
/*     i32 point = 0; */
/*     i32 occupied_sectors = ceil(size / (f32)SECTOR_SIZE); */
/*     for (int i = 0; i < occupied_sectors; i++) { */
/*         u8 sector_data[SECTOR_SIZE]; */
/*         read_sector(f, sector + i, sector_data); */
/*         memcpy(file_data, sector_data, SECTOR_SIZE * sizeof(u8)); // copy 4 floats from x to total[0]...total[3] */
/*         point += SECTOR_SIZE; */
/*     } */
/*     return (file_t){.data=file_data, .len=point}; */
/* } */

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



void read_mesh(FILE *f, int sector, mesh_t *mesh) {
    fseek(f, sector * SECTOR_LEN, SEEK_SET);
    fseek(f, 0x40, SEEK_CUR);

    if (read_u32(f) != 196){
        printf("invalid mesh mesh file\n");
        return;
    }

    // 0x80 is 196 - 0x40 (+4). Always the primary mesh location
    fseek(f, 0x80, SEEK_CUR);

    u16 N = read_u16(f);
    u16 P = read_u16(f);
    u16 Q = read_u16(f);
    u16 R = read_u16(f);

    if (N > 512 || P > 768 || Q > 64 || R > 256) {
        printf("invalid number of polygons N:%i, P:%i, Q:%i, R:%i\n", N, P, Q, R);
        return;
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
        printf("invalid num vertices got: %d, want: %d\n", index, (N*3)+(P*3*2)+(Q*3)+(R*3*2));
        return;
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

    read_palette(f, sector, mesh);
    read_lights(f, sector, mesh);

    mesh->center_transform = mesh_center_transform(mesh);

    mesh->is_mesh_valid = true;
    return;
}

vec3 mesh_center_transform(mesh_t *mesh) {
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

void read_palette(FILE *f, int sector, mesh_t *mesh) {
    // 16 palettes of 16 colors of 4 bytes

    // Seek to beginning of file
    fseek(f, sector * SECTOR_LEN, SEEK_SET);
    // Seek to header pointer
    fseek(f, 0x44, SEEK_CUR);

    // Read palette header pointer
    u32 intra_file_ptr = read_u32(f);

    // Seek to intra file palette location
    fseek(f, sector * SECTOR_LEN, SEEK_SET);
    fseek(f, intra_file_ptr, SEEK_CUR);

    for (int i = 0; i < 16 * 16 * 4; i = i + 4) {
        vec4 c = read_rgb15(f);
        mesh->palette[i+0] = c.x;
        mesh->palette[i+1] = c.y;
        mesh->palette[i+2] = c.z;
        mesh->palette[i+3] = c.w;
    }
}

vec4 read_rgb15(FILE *f) {
        u16 val = read_u16(f);
        u8 a = val == 0 ? 0x00 : 0xFF;
        u8 b = (val & 0x7C00) >> 7; // 0b0111110000000000
        u8 g = (val & 0x03E0) >> 2; // 0b0000001111100000
        u8 r = (val & 0x001F) << 3; // 0b0000000000011111
        return (vec4){r,g,b,a};
}

void read_lights(FILE *f, int sector, mesh_t *mesh) {
    // Seek to beginning of file
    fseek(f, sector * SECTOR_LEN, SEEK_SET);
    // Seek to header pointer
    fseek(f, 0x64, SEEK_CUR);

    // Read palette header pointer
    u32 intra_file_ptr = read_u32(f);

    // Seek to intra file palette location
    fseek(f, sector * SECTOR_LEN, SEEK_SET);
    fseek(f, intra_file_ptr, SEEK_CUR);

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

    vec4 c = read_rgb15(f);
    mesh->ambient_light = (vec3){c.x, c.y, c.z};
}

void read_texture(FILE *f, int sector, mesh_t *mesh) {
    fseek(f, sector * SECTOR_LEN, SEEK_SET);

    u8 raw_pixels[TEXTURE_RAW_SIZE];
    fread(&raw_pixels, TEXTURE_RAW_SIZE, 1, f);

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

    mesh->is_texture_valid = true;

    // Scaling the RGB values from 0-16 to 0-255 so they are visible in ImGUI.
    for (int i = 0; i < TEXTURE_NUM_BYTES; i = i + 4) {
        mesh->texture_display[i+0] = mesh->texture[i+0] * 17;
        mesh->texture_display[i+1] = mesh->texture[i+1] * 17;
        mesh->texture_display[i+2] = mesh->texture[i+2] * 17;
        mesh->texture_display[i+3] = 255;
    }

    return;
}

void read_gns(FILE *f, int sector, gns_t *gns) {
    fseek(f, sector * SECTOR_LEN, SEEK_SET);

    while (true) {
        u16 header_unknown = read_u16(f);
        if (header_unknown != 0x22 && header_unknown != 0x30 && header_unknown != 0x70) {
            printf("invalid header value: %x\n", header_unknown);
            return;
        }

        u8 room_arrangement = read_u8(f); (void)room_arrangement;
        u8 time_weather = read_u8(f);     (void)time_weather;
        u16 file_type = read_u16(f);
        (void)read_u16(f); // padding
        u16 file_sector = read_u16(f);
        (void)read_u16(f); // padding
        u32 file_length = read_u32(f);    (void) file_length;
        (void)read_u32(f); // padding

        if (file_type == FFTRecordEnd) {
            break;
        }

        gns->records[gns->num_records].file_type = file_type;
        gns->records[gns->num_records].file_sector = file_sector,
        gns->num_records++;

        if (gns->num_records >= MAX_GNS_RECORDS) {
            printf("invalid number of gns records\n");
            return;
        }
    }
    gns->is_valid = true;
    return;
}

u8 read_u8(FILE *f)
{
    u8 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

u16 read_u16(FILE *f)
{
    u16 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

u32 read_u32(FILE *f)
{
    u32 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

i8 read_i8(FILE *f)
{
    i8 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

i16 read_i16(FILE *f)
{
    i16 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

i32 read_i32(FILE *f)
{
    i32 value;
    fread(&value, sizeof(value), 1, f);
    return value;
}

f32 read_f1x3x12(FILE *f)
{
    i16 value = read_i16(f);
    return (f32)value / 4096.0;
}

vec3 read_position(FILE *f)
{
    f32 x = read_i16(f);
    f32 y = read_i16(f);
    f32 z = read_i16(f);

    x =  x / 100.0;
    y = -y / 100.0;
    z = -z / 100.0;

    return (vec3){ x, y, z };
}

vec3 read_normal(FILE *f)
{
    f32 x = read_f1x3x12(f);
    f32 y = read_f1x3x12(f);
    f32 z = read_f1x3x12(f);

    y = -y;
    z = -z;

    return (vec3){ x, y, z };
}
