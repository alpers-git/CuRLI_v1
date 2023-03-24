#version 450 core

layout (triangles, equal_spacing, ccw) in;

vec4 interpolate(vec4 v0, vec4 v1, vec4 v2)
{
    return v0 *gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}

// in VS_OUT ts_in[];
// out VS_OUT ts_out[];

void main()
{
    // Calculate the barycentric coordinates of the current vertex
    gl_Position = interpolate(  gl_in[0].gl_Position, 
                                gl_in[1].gl_Position, 
                                gl_in[2].gl_Position);
}