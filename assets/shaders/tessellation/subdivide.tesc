#version 450 core
precision highp float;

layout(vertices = 3) out;
in vec2 tex_coords_tesc[];
out vec2 tex_coords_tese[];

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
    tex_coords_tese[gl_InvocationID] = tex_coords_tesc[gl_InvocationID];
}