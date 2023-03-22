#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 texc;


layout (location = 3) out vec3 v_space_norm;
layout (location = 4) out vec3 v_space_pos;
layout (location = 5) out vec2 tex_coord;
layout (location = 6) out vec3 w_space_pos;
layout (location = 7) out vec3 w_space_norm;
layout (location = 8) out vec4 lv_space_pos;


uniform mat4 to_screen_space; // mvp
uniform mat4 to_view_space; //mv
uniform mat4 to_world_space; //m

uniform mat3 normals_to_world_space;
uniform mat3 normals_to_view_space;
uniform vec3 camera_pos;
uniform int mirror_reflection = 0;

const mat4 scale_bias = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.5, 0.5, 0.5, 1.0));

void main() {
    gl_Position = to_screen_space * vec4(pos, 1.0);
    tex_coord = texc;
    v_space_norm = normals_to_view_space * norm;
    v_space_pos = (to_view_space * vec4(pos, 1.0)).xyz;

    w_space_pos = (to_world_space * vec4(pos, 1.0)).xyz;
    w_space_norm = normals_to_world_space * norm;
    
    if(mirror_reflection==1)
    {
        tex_coord = (scale_bias * gl_Position).xy / (scale_bias * gl_Position).w;
        //clamp between 0 and 1
        tex_coord = clamp(tex_coord, 0.0, 1.0);
    }
}
     