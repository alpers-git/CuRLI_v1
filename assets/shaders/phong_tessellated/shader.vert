#version 450

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec2 texc;


layout(location = 3) out vec3 v_space_norm_tesc;
layout(location = 4) out vec3 v_space_pos_tesc;
layout(location = 5) out vec2 tex_coord_tesc;
layout(location = 6) out vec3 w_space_pos_tesc;
layout(location = 7) out vec3 w_space_norm_tesc;


uniform mat4 to_screen_space; // mvp
uniform mat4 to_view_space; //mv
uniform mat4 to_world_space; //m

uniform mat3 normals_to_world_space;
uniform mat3 normals_to_view_space;
uniform vec3 camera_pos;
uniform int mirror_reflection = 0;
uniform int shading_mode;

const mat4 scale_bias = mat4(vec4(0.5, 0.0, 0.0, 0.0), vec4(0.0, 0.5, 0.0, 0.0), vec4(0.0, 0.0, 0.5, 0.0), vec4(0.5, 0.5, 0.5, 1.0));

void main() {
    if (shading_mode == 2)
        gl_Position = to_screen_space * vec4(pos, 1.0);
    else
        gl_Position = /*to_screen_space */ vec4(pos, 1.0);
    tex_coord_tesc = texc;
    v_space_norm_tesc = normals_to_view_space * norm;
    v_space_pos_tesc = (to_view_space * vec4(pos, 1.0)).xyz;

    w_space_pos_tesc = (to_world_space * vec4(pos, 1.0)).xyz;
    w_space_norm_tesc = normals_to_world_space * norm;
    
    if(mirror_reflection==1)
    {
        tex_coord_tesc = (scale_bias * gl_Position).xy / (scale_bias * gl_Position).w;
        //clamp between 0 and 1
        tex_coord_tesc = clamp(tex_coord_tesc, 0.0, 1.0);
    }
}
     