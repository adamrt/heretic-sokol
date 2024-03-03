#include <math.h>
#include <string.h>

#include "maths.h"

inline vec3 vec3_add(vec3 a, vec3 b)
{
    return (vec3) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

inline vec3 vec3_mulf(vec3 v, f32 f)
{
    return (vec3) {
        v.x * f,
        v.y * f,
        v.z * f,
    };
}

inline vec3 vec3_divf(vec3 v, f32 f)
{
    return (vec3) {
        v.x / f,
        v.y / f,
        v.z / f,
    };
}

inline f32 vec3_length(vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline void vec3_normalize(vec3* v)
{
    const f32 length = vec3_length(*v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

inline vec3 vec3_normalized(vec3 v)
{
    vec3_normalize(&v);
    return v;
}

inline f32 vec3_dot(vec3 a, vec3 b)
{
    f32 p = 0;
    p += a.x * b.x;
    p += a.y * b.y;
    p += a.z * b.z;
    return p;
}

inline vec3 vec3_cross(vec3 a, vec3 b)
{
    return (vec3) {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
}

inline vec4 vec3_to_vec4(vec3 v)
{
    return (vec4) {
        v.x,
        v.y,
        v.z,
        1.0f,
    };
}

inline mat4 mat4_identity(void)
{
    mat4 result;
    memset(result.data, 0, sizeof(f32) * 16);
    result.data[0] = 1.0f;
    result.data[5] = 1.0f;
    result.data[10] = 1.0f;
    result.data[15] = 1.0f;
    return result;
}

inline mat4 mat4_mul(mat4 a, mat4 b)
{
    mat4 result = mat4_identity();

    const f32* a_ptr = a.data;
    const f32* b_ptr = b.data;
    f32* dst_ptr = result.data;

    for (i32 i = 0; i < 4; ++i) {
        for (i32 j = 0; j < 4; ++j) {
            *dst_ptr = a_ptr[0] * b_ptr[0 + j] + a_ptr[1] * b_ptr[4 + j] + a_ptr[2] * b_ptr[8 + j] + a_ptr[3] * b_ptr[12 + j];
            dst_ptr++;
        }
        a_ptr += 4;
    }
    return result;
}

inline mat4 mat4_orthographic(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    mat4 result = mat4_identity();

    f32 lr = 1.0f / (left - right);
    f32 bt = 1.0f / (bottom - top);
    f32 nf = 1.0f / (near - far);

    result.data[0] = -2.0f * lr;
    result.data[5] = -2.0f * bt;
    result.data[10] = 2.0f * nf;

    result.data[12] = (left + right) * lr;
    result.data[13] = (top + bottom) * bt;
    result.data[14] = (far + near) * nf;
    return result;
}

inline mat4 mat4_perspective(f32 fov_radians, f32 aspect, f32 near, f32 far)
{
    f32 half_tan_fov = tanf(fov_radians * 0.5f);
    mat4 result;
    memset(result.data, 0, sizeof(f32) * 16);
    result.data[0] = 1.0f / (aspect * half_tan_fov);
    result.data[5] = 1.0f / half_tan_fov;
    result.data[10] = -((far + near) / (far - near));
    result.data[11] = -1.0f;
    result.data[14] = -((2.0f * far * near) / (far - near));
    return result;
}

inline mat4 mat4_look_at(vec3 position, vec3 target, vec3 up)
{
    mat4 result;
    vec3 z_axis;
    z_axis.x = target.x - position.x;
    z_axis.y = target.y - position.y;
    z_axis.z = target.z - position.z;

    z_axis = vec3_normalized(z_axis);
    vec3 x_axis = vec3_normalized(vec3_cross(z_axis, up));
    vec3 y_axis = vec3_cross(x_axis, z_axis);

    result.data[0] = x_axis.x;
    result.data[1] = y_axis.x;
    result.data[2] = -z_axis.x;
    result.data[3] = 0;
    result.data[4] = x_axis.y;
    result.data[5] = y_axis.y;
    result.data[6] = -z_axis.y;
    result.data[7] = 0;
    result.data[8] = x_axis.z;
    result.data[9] = y_axis.z;
    result.data[10] = -z_axis.z;
    result.data[11] = 0;
    result.data[12] = -vec3_dot(x_axis, position);
    result.data[13] = -vec3_dot(y_axis, position);
    result.data[14] = vec3_dot(z_axis, position);
    result.data[15] = 1.0f;

    return result;
}

inline mat4 mat4_translation(vec3 position)
{
    mat4 result = mat4_identity();
    result.data[12] = position.x;
    result.data[13] = position.y;
    result.data[14] = position.z;
    return result;
}

inline mat4 mat4_scale(vec3 scale)
{
    mat4 result = mat4_identity();
    result.data[0] = scale.x;
    result.data[5] = scale.y;
    result.data[10] = scale.z;
    return result;
}

//
// Utilities
//

inline f32 radians(f32 degrees) { return degrees * DEG2RAD_MUL; }

inline f32 clamp(f32 value, f32 min, f32 max)
{
    if (value <= min) {
        return min;
    }
    if (value >= max) {
        return max;
    }
    return value;
}
