#pragma once

#include <stdint.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef float    f32;
typedef double   f64;

typedef struct {
    f32 x, y, z;
    f32 u, v;
} vertex_t;
