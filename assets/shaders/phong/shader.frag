#version 430
precision mediump float;

layout (location = 2) in vec3 f_norm;
layout (location = 3) in vec4 f_pos;

uniform struct {
     vec3 ka;
     vec3 kd;
     vec3 ks;
     float shininess;
}material;
out vec4 color;

void main() {
     vec3 f_norm_normalized = clamp(normalize(f_norm),0,1);
     vec3 light_dir = normalize(vec3(0,1,0));
     vec3 h = normalize(light_dir - f_pos.xyz); //half vector

     float cos_theta = dot(light_dir, f_norm_normalized);
     if(cos_theta < 0) {
          color = vec4(material.ka * 0.1,1);
     }
     else {
          vec3 diffuse = material.kd * cos_theta;
          vec3 ambient = material.ka * 0.1;
          vec3 specular= material.ks * pow(dot(h, f_norm_normalized), material.shininess);
          color = vec4(specular + diffuse + ambient, 1);
     }
}