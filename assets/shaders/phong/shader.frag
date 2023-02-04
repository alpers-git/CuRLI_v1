#version 430

out vec4 color;
layout (location = 2) in vec3 f_norm;
void main() {
     vec3 f_norm_normalized = normalize(f_norm);
     color = vec4(f_norm_normalized, 1.0);
}