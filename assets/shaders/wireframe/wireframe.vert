#version 450
precision highp float;

in vec3 pos;

out VS_OUT {
    vec3 color;
} vs_out;


uniform mat4 to_screen_space; // mvp

void main() {
    vs_out.color = vec3(1.0, 1.0, 1.0);
    gl_Position = to_screen_space * vec4(pos, 1.0);
}