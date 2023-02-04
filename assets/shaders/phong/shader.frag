#version 430
precision mediump float;

layout (location = 2) in vec3 f_norm;
uniform struct {
     vec3 ambient;
     vec3 diffuse;
     vec3 specular;
     float shininess;
}material;
out vec4 color;

void main() {
     vec3 f_norm_normalized = clamp(normalize(f_norm),0,1);
     color = vec4(dot(vec3(0,1,0), f_norm_normalized) * material.diffuse, 1);
}