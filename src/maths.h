#pragma once

#include "defines.h"

#define PI 3.14159265358979323846f
#define DEG2RAD_MUL (PI / 180.0f)

typedef struct { f32 x, y; } vec2;
typedef struct { f32 x, y, z; } vec3;
typedef struct { f32 x, y, z, w; } vec4;
typedef struct { f32 data[16]; } mat4;

vec3 vec3_add(vec3 a, vec3 b);
vec3 vec3_mulf(vec3 v, f32 f);
vec3 vec3_divf(vec3 a, f32 f);
vec3 vec3_normalized(vec3 v);
void vec3_normalize(vec3* v);
vec3 vec3_cross(vec3 a, vec3 b);
f32 vec3_dot(vec3 a, vec3 b);
f32 vec3_length(vec3 v);

mat4 mat4_identity(void);
mat4 mat4_mul(mat4 a, mat4 b);
mat4 mat4_look_at(vec3 position, vec3 target, vec3 up);
mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far);
mat4 mat4_perspective(f32 fov_radians, f32 aspect, f32 near, f32 far);
mat4 mat4_translation(vec3 position);
mat4 mat4_scale(vec3 scale);

f32 radians(f32 degrees);
f32 clamp(f32 value, f32 min, f32 max);
