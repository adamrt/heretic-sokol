@vs vs
uniform vs_params {
    vec4 color;
};

in vec3 position;
out vec4 ourColor;

void main()
{
    gl_Position = vec4(position.xyz, 1.0);
    ourColor = color;
}
@end

@fs fs
in  vec4 ourColor;
out vec4 FragColor;

void main()
{
    FragColor = ourColor;
}
@end

@program basic vs fs
