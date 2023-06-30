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

uniform sampler2D tex;

void main()
{
    FragColor = texture(tex, uv);;
}
@end

@program basic vs fs
