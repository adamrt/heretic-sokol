@ctype mat4 mat4_t
@ctype vec3 vec3_t

@vs vs_basic
uniform vs_basic_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec3 aPos;
in vec3 aNormal;

out vec3 Normal;
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

    // FragPos = vec3(model * vec4(aPos, 1.0));
}
@end

@fs fs_basic
uniform fs_basic_params {
    vec3 lightColor;
    vec3 lightPos;
};

out vec4 FragColor;

in vec3 Normal;
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
    vec3 objectColor = vec3(1.0f, 0.5f, 0.31f);
    vec3 result = (ambient + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
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
