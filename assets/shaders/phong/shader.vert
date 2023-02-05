#version 430

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;

layout (location = 2) out vec3 v_space_norm;
layout (location = 3) out vec3 l;


layout(location = 0) uniform mat4 to_screen_space; // mvp
layout(location = 1) uniform mat4 to_view_space; //mv
layout(location = 2) uniform mat3 normals_to_view_space; 
layout(location = 3) uniform vec3 light_position;
//layout(location = 4) float light_intensity;

void main() {
    gl_Position = to_screen_space * vec4(pos, 1.0);
    v_space_norm = normals_to_view_space * norm;
    vec3 v_space_pos = (to_view_space * vec4(pos, 1.0)).xyz;

    l = normalize( (to_view_space * vec4(light_position, 1)).xyz - v_space_pos);
}
     