@ctype mat4 mat4_t
@ctype vec3 vec3_t

@vs vs_basic
uniform vs_basic_params {
    mat4 u_model;
    mat4 u_view;
    mat4 u_projection;
};

in vec3 a_pos;
in vec3 a_normal;
in vec3 a_uv;

out vec3 v_pos;
out vec3 v_normal;
out vec3 v_uv;

void main()
{
    v_pos = vec3(u_model * vec4(a_pos, 1.0));
    v_normal = mat3(transpose(inverse(u_model))) * a_normal;
    v_uv = a_uv;
    gl_Position = u_projection * u_view * u_model * vec4(a_pos, 1.0);
}
@end

@fs fs_basic
uniform fs_basic_params {
    int  u_draw_mode;
    vec3 u_ambient_color;
    vec3 u_light_color;
    vec3 u_light_pos;
};

uniform sampler2D u_tex;
uniform sampler2D u_palette;

in vec3 v_pos;
in vec3 v_normal;
in vec3 v_uv;

out vec4 frag_color;

void main()
{
    // Ambient Light
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * u_ambient_color;

    // Diffuse Light
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(u_light_pos - v_pos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_light_color;

    // Texture
    if (u_draw_mode == 0) {
        vec4 tex_color = texture(u_tex, vec2(v_uv.xy)) * 255.0;
        uint palette_pos = uint(v_uv.z) * 16u + uint(tex_color.r);
        vec4 color = texture(u_palette, vec2(float(palette_pos) / 255.0, 0.0));
        if (color.a < 0.5)
            discard;
        frag_color = vec4(ambient + diffuse, 1.0) * color;
    } else if (u_draw_mode == 1) {
        vec4 color = vec4(v_normal, 1.0);
        frag_color = vec4(ambient + diffuse, 1.0) * color;
    } else {
        vec4 color = vec4(0.6, 0.2, 0.0, 1.0);
        frag_color = vec4(ambient + diffuse, 1.0) * color;
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
