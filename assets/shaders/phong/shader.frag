#version 430
precision mediump float;

layout (location = 2) in vec3 f_norm;
layout (location = 3) in vec3 f_pos;
layout (location = 4) in vec3 L;

layout(location = 1) uniform mat4 mv;
//layout(location = 3) vec3 light_position;
layout(location = 4) uniform float light_intensity;
uniform struct {
     vec3 ka;
     vec3 kd;
     vec3 ks;
     float shininess;
}material;

out vec4 color;

void main() {
     vec3 f_norm = normalize(f_norm);
     vec3 L = normalize(L); //light vector
     vec3 H = normalize(L -f_pos); //half vector

     float cos_theta = dot(L, f_norm);
     if(cos_theta < 0) {
          color = vec4(material.ka, 1);
     }
     else {
          vec3 diffuse = material.kd * cos_theta;
          vec3 ambient = material.ka;
          vec3 specular= material.ks * pow(max(dot(H, f_norm),0), material.shininess);
          color = vec4(light_intensity * (specular + diffuse) + ambient, 1);
     }
}