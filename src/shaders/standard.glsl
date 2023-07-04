@ctype mat4 mat4_t
@ctype vec3 vec3_t

@vs vs_basic
uniform vs_basic_params {
    float draw_mode;
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;

out vec3 Normal;
out vec2 UV;
out vec3 FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    // The position of the worldspace fragment to calculate light.
    FragPos = vec3(model * vec4(aPos, 1.0));
    // This transforms the normal by the normal matrix. The normal
    // matrix is created here, but its expensive. This should could be
    // done on the CPU and sent in as a uniform.
    Normal = mat3(transpose(inverse(model))) * aNormal;
    UV = aTexCoords;
}
@end

@fs fs_basic
uniform fs_basic_params {
    vec3 lightColor;
    vec3 lightPos;
    int draw_mode;

};
uniform sampler2D texture1;

out vec4 FragColor;

in vec3 Normal;
in vec2 UV;
in vec3 FragPos;

void main()
{
    // Ambient Light
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse Light
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Texture
    if (draw_mode == 0) {
        vec4 tex = texture(texture1, UV);
        FragColor = vec4(ambient + diffuse, 1.0) * tex;
    } else if (draw_mode == 1) {
        vec4 color = vec4(Normal, 1.0);
        FragColor = vec4(ambient + diffuse, 1.0) * color;
    } else {
        vec4 color = vec4(0.6, 0.2, 0.0, 1.0);
        FragColor = vec4(ambient + diffuse, 1.0) * color;
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
out vec4 FragColor;

void main() {
    FragColor = vec4(1.0);      // set all 4 vector values to 1.0
}
@end

@program basic vs_basic fs_basic
@program light vs_light fs_light
