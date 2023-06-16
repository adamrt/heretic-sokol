@vs vs_basic

layout(location=0) in vec4 position;
layout(location=1) in vec4 color0;

out vec4 color;

void main() {
    gl_Position = position;
    color = color0;
}
@end

@fs fs_basic
in vec4 color;
out vec4 frag_color;

void main() {
    frag_color = color;
}
@end

@program basic vs_basic fs_basic
