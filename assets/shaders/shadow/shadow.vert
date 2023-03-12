#version 450

layout (location = 0) in vec3 pos;

uniform mat4 to_screen_space; // mvp

void main() {
    gl_Position = to_screen_space * vec4(pos, 1.0);
}