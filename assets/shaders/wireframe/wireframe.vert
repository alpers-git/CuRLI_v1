#version 450
precision highp float;

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 texc;

out vec2 tex_coords_tesc;

uniform mat4 to_screen_space; // mvp

void main() {
    gl_Position = to_screen_space * vec4(pos, 1.0);
    tex_coords_tesc = texc;
}