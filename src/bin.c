#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "bin.h"

// read_sector reads a sector to `out_bytes`.
static b8 read_sector(FILE *f, i32 sector, u8 *out_bytes) {
    i32 seek_to = (sector * SECTOR_SIZE_RAW) + SECTOR_HEADER_SIZE;
    if (fseek(f, seek_to, SEEK_SET) != 0) {
        return false;
    }
    size_t n = fread(out_bytes, sizeof(u8), SECTOR_SIZE, f);
    b8 success = n == SECTOR_SIZE;
    return success;
}

// read_file reads an entire file, sector by sector.
b8 read_file(FILE *f, i32 sector, i32 size, file_t *out_file)  {
    i32 occupied_sectors = ceil((f32)size / (f32)SECTOR_SIZE);
    for (i32 i = 0; i < occupied_sectors; i++) {
        u8 sector_data[SECTOR_SIZE];
        read_sector(f, sector + i, sector_data);
        memcpy(out_file->data+out_file->len, sector_data, SECTOR_SIZE);
        out_file->len += SECTOR_SIZE;
    }
    out_file->offset = 0;
    return true;
}

u8 read_u8(file_t *f) {
    u8 value;
    memcpy(&value, &f->data[f->offset], sizeof(u8));
    f->offset += sizeof(u8);
    return value;
}

u16 read_u16(file_t *f) {
    u16 value;
    memcpy(&value, &f->data[f->offset], sizeof(u16));
    f->offset += sizeof(u16);
    return value;
}

u32 read_u32(file_t *f) {
    u32 value;
    memcpy(&value, &f->data[f->offset], sizeof(u32));
    f->offset += sizeof(u32);
    return value;
}

i8 read_i8(file_t *f) {
    i8 value;
    memcpy(&value, &f->data[f->offset], sizeof(i8));
    f->offset += sizeof(i8);
    return value;
}

i16 read_i16(file_t *f) {
    i16 value;
    memcpy(&value, &f->data[f->offset], sizeof(i16));
    f->offset += sizeof(i16);
    return value;
}

i32 read_i32(file_t *f) {
    i32 value;
    memcpy(&value, &f->data[f->offset], sizeof(i32));
    f->offset += sizeof(i32);
    return value;
}
