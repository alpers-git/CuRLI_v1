#version 450 core
precision highp float;

//------------ Variying ------------
//============== in ================
layout(location = 3) in vec3 v_space_norm_tesc[];
layout(location = 4) in vec3 v_space_pos_tesc[];
layout(location = 5) in vec2 tex_coord_tesc[];
layout(location = 6) in vec3 w_space_pos_tesc[];
layout(location = 7) in vec3 w_space_norm_tesc[];

//============== out ===============
layout(vertices = 3) out;
layout(location = 8) out vec3 v_space_norm_tese[];
layout(location = 9) out vec3 v_space_pos_tese[];
layout(location = 10) out vec2 tex_coord_tese[];
layout(location = 11) out vec3 w_space_pos_tese[];
layout(location = 12) out vec3 w_space_norm_tese[];

uniform int tessellation_level;
void main()
{
    // Set the level of tessellation to 4
    gl_TessLevelOuter[0] = tessellation_level;
    gl_TessLevelOuter[1] = tessellation_level;
    gl_TessLevelOuter[2] = tessellation_level;

    gl_TessLevelInner[0] = tessellation_level;

    // Pass the input vertices to the output patch
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    v_space_norm_tese[gl_InvocationID] = v_space_norm_tesc[gl_InvocationID]; 
    v_space_pos_tese[gl_InvocationID] = v_space_pos_tesc[gl_InvocationID]; 
    tex_coord_tese[gl_InvocationID] = tex_coord_tesc[gl_InvocationID];
    w_space_pos_tese[gl_InvocationID] = w_space_pos_tesc[gl_InvocationID]; 
    w_space_norm_tese[gl_InvocationID] = w_space_norm_tesc[gl_InvocationID]; 
}