#version 450 core
precision highp float;

layout (triangles, equal_spacing, ccw) in;

in vec2 tex_coords_tese[];

vec4 interpolate(vec4 v0, vec4 v1, vec4 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}

vec2 interpolate(vec2 v0, vec2 v1, vec2 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}

uniform float displacement_multiplier;
uniform sampler2D displacement_map;
uniform mat4 to_screen_space;

void main()
{
    vec2 tex_coords = interpolate(tex_coords_tese[0], tex_coords_tese[1], tex_coords_tese[2]);

    vec3 displacement = texture(displacement_map, tex_coords).xyz;
    gl_Position = to_screen_space * (interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position) +
                        vec4(0, 0, texture(displacement_map, tex_coords).y * displacement_multiplier, 1.0));
}
