// This file contains ways to read BIN files.
#pragma once
#include <stdio.h>

#include "defines.h"

 // Max size of any resource file.
#define FILE_MAX_SIZE 131072
#define SECTOR_SIZE 2048
#define SECTOR_SIZE_RAW 2352
#define SECTOR_HEADER_SIZE 24

// file_t represents a file in a BIN file.
typedef struct {
    u8 data[FILE_MAX_SIZE];
    u64 len;
    u64 offset;
} file_t;

b8 read_file(FILE *f, i32 sector, i32 size, file_t *out_file);

u8 read_u8(file_t *f);
u16 read_u16(file_t *f);
u32 read_u32(file_t *f);
i8 read_i8(file_t *f);
i16 read_i16(file_t *f);
i32 read_i32(file_t *f);
