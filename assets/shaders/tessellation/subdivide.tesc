#version 450 core

layout(vertices = 3) out;

// in VS_OUT ts_in[];
// out VS_OUT ts_out[];

void main()
{
    // Set the level of tessellation to 4
    gl_TessLevelOuter[0] = 2;
    gl_TessLevelOuter[1] = 2;
    gl_TessLevelOuter[2] = 2;

    gl_TessLevelInner[0] = 3;


    // ts_out[gl_InvocationID].position = ts_in[gl_InvocationID].position;
    // ts_out[gl_InvocationID] = ts_in[gl_InvocationID];

    // Pass the input vertices to the output patch
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}