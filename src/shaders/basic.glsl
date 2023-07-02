@ctype mat4 mat4_t
@ctype vec3 vec3_t

@vs vs_basic
uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 projection;
};

in vec3 aPos;
in vec2 aTexCoord;

out vec2 uv;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    uv = aTexCoord;
}
@end

@fs fs_basic
uniform fs_basic_params {
    vec3 lightColor;
};

out vec4 FragColor;
in vec2 uv;

uniform sampler2D texture1;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
    vec4 result = texture(texture1, uv) * vec4(ambient, 1.0);
    FragColor = result;
}
@end

@vs vs_light
uniform vs_params {
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
