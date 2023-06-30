@vs vs_basic
in vec3 position;

void main()
{
    gl_Position = vec4(position.xyz, 1.0);
}
@end

@fs fs_basic
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
@end

@program basic vs_basic fs_basic
