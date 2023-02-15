#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

layout (location = 2) out vec3 v_space_norm;
layout (location = 3) out vec3 v_space_pos;


layout(location = 0) uniform mat4 to_screen_space; // mvp
layout(location = 1) uniform mat4 to_view_space; //mv
layout(location = 2) uniform mat3 normals_to_view_space; 

void main() {
    gl_Position = to_screen_space * vec4(pos, 1.0);
    v_space_norm = normals_to_view_space * norm;
    v_space_pos = (to_view_space * vec4(pos, 1.0)).xyz;
}
     