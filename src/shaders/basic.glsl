@vs vs
uniform vs_params {
    float uSlider;
};

in vec3 aPos;
in vec2 aTexCoord;

out vec2 uv;
out float slider;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    uv = aTexCoord;
    slider = uSlider;
}
@end





@fs fs
out vec4 FragColor;

in vec2 uv;
in float slider;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    FragColor = mix(texture(texture1, uv), texture(texture2, uv), slider);
}
@end

@program basic vs fs
