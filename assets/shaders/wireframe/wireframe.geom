#version 450
precision highp float;

layout (triangles, invocations = 3) in;
layout (line_strip, max_vertices = 5) out;

void main()
{
    gl_Position = gl_in[gl_InvocationID].gl_Position -
        vec4(0.0, 0.0, 3.0, 0.0);
    EmitVertex();
    
    gl_Position = gl_in[(gl_InvocationID + 1) % 3].gl_Position -
        vec4(0.0, 0.0, 3.0, 0.0);
    EmitVertex();
    EndPrimitive();
}