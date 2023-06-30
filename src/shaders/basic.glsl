@vs vs
in vec3 aPos;
in vec2 aTexCoord;

out vec2 uv;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    uv = aTexCoord;
}
@end

@fs fs
out vec4 FragColor;

in vec2 uv;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, uv), texture(texture2, uv), 0.2);
}
@end

@program basic vs fs
