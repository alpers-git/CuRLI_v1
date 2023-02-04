#version 430

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec3 v_norm;

layout (location = 2) out vec3 f_norm;
layout (location = 3) out vec4 f_pos;


layout(location = 0) uniform mat4 mvp;
layout(location = 1) uniform mat4 mv;
layout(location = 2) uniform mat3 normal_mat;

void main() {
    gl_Position = mvp * vec4(v_pos, 1.0);
    f_norm = normal_mat * v_norm;
    f_pos = mv * vec4(v_pos, 1.0);
}
     