#version 450 core
precision highp float;


//------------ Variying ------------
//============== in ================
layout (triangles, equal_spacing, ccw) in;
layout(location = 8)  in vec3 v_space_norm_tese[];
layout(location = 9)  in vec3 v_space_pos_tese[];
layout(location = 10) in vec2 tex_coord_tese[];
layout(location = 11) in vec3 w_space_pos_tese[];
layout(location = 12) in vec3 w_space_norm_tese[];

//============== out ===============
layout(location = 13)  out vec3 v_space_norm;
layout(location = 14)  out vec3 v_space_pos;
layout(location = 15) out vec2 tex_coord;
layout(location = 16) out vec3 w_space_pos;
layout(location = 17) out vec3 w_space_norm;

vec4 interpolate(vec4 v0, vec4 v1, vec4 v2)
{
    return v0 * gl_TessCoord.x + v1 * gl_TessCoord.y + v2 * gl_TessCoord.z;
}

vec3 interpolate(vec3 v0, vec3 v1, vec3 v2)
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
    tex_coord = interpolate(tex_coord_tese[0], tex_coord_tese[1], tex_coord_tese[2]);

    v_space_norm = interpolate(v_space_norm_tese[0], v_space_norm_tese[1], v_space_norm_tese[2]);
    v_space_pos = interpolate(v_space_pos_tese[0], v_space_pos_tese[1], v_space_pos_tese[2]);
    tex_coord = interpolate(tex_coord_tese[0], tex_coord_tese[1], tex_coord_tese[2]);
    w_space_pos = interpolate(w_space_pos_tese[0], w_space_pos_tese[1], w_space_pos_tese[2]);
    w_space_norm = interpolate(w_space_norm_tese[0], w_space_norm_tese[1], w_space_norm_tese[2]);

    vec3 displacement = texture(displacement_map, tex_coord).xyz;
    gl_Position = to_screen_space * (interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position) +
                  vec4(displacement * displacement_multiplier, 1.0));

}
