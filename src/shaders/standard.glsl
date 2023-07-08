// These are for our custom math types, which are structs, not arrays.
@ctype mat4 mat4
@ctype vec4 vec4
@ctype vec3 vec3

@vs vs_basic
uniform vs_basic_params {
    mat4 u_model;
    mat4 u_view;
    mat4 u_projection;
};

in vec3  a_pos;
in vec3  a_normal;
in vec2  a_uv;
in float a_palette;

out vec3  v_pos;
out vec3  v_normal;
out vec2  v_uv;
out float v_palette;

void main()
{
    v_pos = vec3(u_model * vec4(a_pos, 1.0));
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_uv = a_uv;
    v_palette = a_palette;
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}
@end

@fs fs_basic

uniform fs_dir_lights {
    vec4 position[3];
    vec4 color[3];
} dir_lights;

uniform fs_basic_params {
    int  u_draw_mode;
    vec3 u_ambient_color;
};

uniform sampler2D u_tex;
uniform sampler2D u_palette;

in vec3  v_pos;
in vec3  v_normal;
in vec2  v_uv;
in float v_palette;

out vec4 frag_color;

void main()
{
    // Ambient Light
    float ambientStrength = 0.2;
    vec4 ambient = ambientStrength * vec4(u_ambient_color, 1.0);

    vec3 norm = normalize(v_normal);

    vec4 diffuse_light_sum = vec4(0.0, 0.0, 0.0, 1.0);

    for (int i = 0; i < 3; i++) {
        vec3 direction = normalize(dir_lights.position[i].xyz - v_pos.xyz);
        float intensity = max(dot(norm, direction), 0.0);
        diffuse_light_sum += dir_lights.color[i] * intensity;
    }
    vec4 light = ambient * 2.0 + diffuse_light_sum;

    // Draw black for triangles without normals (untextured triangles)
    if (v_normal.x + v_normal.y + v_normal.z + v_uv.x + v_uv.y == 0.0) {
        // Draw black for things without normals and uv coords.
        // The uv coords and normal could actually be 0 so we check them both.
        frag_color = light * vec4(0.1, 0.1, 0.1, 1.0);
        return;
    }

    if (u_draw_mode == 0) {
        // Textured

        // This has to be 256.0 instead of 255 (really 255.1 is fine).
        // And palette_pos needs to be calculated then cast to uint,
        // not casting each to uint then calculating. Otherwise there
        // will be distortion in perspective projection on some gpus.
        vec4 tex_color = texture(u_tex, v_uv) * 256.0;
        uint palette_pos = uint(v_palette * 16 + tex_color.r);
        vec4 color = texture(u_palette, vec2(float(palette_pos) / 255.0, 0.0));
        if (color.a < 0.5)
            discard;
        frag_color = light * color;
    } else if (u_draw_mode == 1) {
        // Normals
        frag_color = vec4(v_normal, 1.0);
    } else {
        // Flat White and lighting
        vec4 color = vec4(0.8f, 0.8f, 0.8f, 1.0f);
        frag_color = light * color;
    }
}
@end

@vs vs_light
uniform vs_light_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec3 aPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
@end

@fs fs_light
uniform fs_light_params {
    vec3 u_light_color;
};

out vec4 FragColor;

void main() {
    FragColor = vec4(u_light_color, 1.0);
}
@end

@program basic vs_basic fs_basic
@program light vs_light fs_light
