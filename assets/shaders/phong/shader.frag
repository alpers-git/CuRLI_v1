#version 430
precision mediump float;

layout (location = 2) in vec3 v_space_norm;
layout (location = 3) in vec3 l;

layout(location = 4) uniform float light_intensity;
uniform struct {
     vec3 ka;
     vec3 kd;
     vec3 ks;
     float shininess;
}material;

out vec4 color;

void main() {
     vec3 v_space_norm = normalize(v_space_norm);
     vec3 l = normalize(l); //light vector
     vec3 h = normalize(l + vec3(0,0,1)); //half vector

     float cos_theta = dot(l, v_space_norm);
     if(cos_theta >= 0)
     {
          vec3 diffuse = material.kd * max(cos_theta,0);
          vec3 ambient = material.ka;
          vec3 specular= material.ks * pow(max(dot(h, v_space_norm),0), material.shininess);
          color = vec4(light_intensity * (specular + diffuse) + ambient, 1);
     }
     else
     {
          color = vec4(material.ka,1);
     }
}