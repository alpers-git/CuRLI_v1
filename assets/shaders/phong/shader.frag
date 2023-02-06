#version 450
precision mediump float;
//------------ Structs ------------
// struct Light{
//      vec3 position;
//      float intensity;
// };

//------------ Variying ------------
layout (location = 2) in vec3 v_space_norm;
layout (location = 3) in vec3 v_space_pos;

//------------ Uniforms ------------
layout(location = 1) uniform mat4 to_view_space; //mv
//layout(location = 3) uniform vec3 light_position;

//layout(location = 4) uniform float light_intensity;
layout(location = 3) uniform struct{
     vec3 position;
     float intensity;
}light;


layout(location = 5) uniform vec3 material_ka;
layout(location = 6) uniform vec3 material_kd;
layout(location = 7) uniform vec3 material_ks;
layout(location = 8) uniform float material_shininess;


out vec4 color;

void main() {
     vec3 v_space_norm = normalize(v_space_norm);
     vec3 l =  normalize(light.position - v_space_pos);//normalize(l); //light vector
     vec3 h = normalize(l + vec3(0,0,1)); //half vector

     float cos_theta = dot(l, v_space_norm);
     if(cos_theta >= 0)
     {
          vec3 diffuse = material_kd * max(cos_theta,0);
          vec3 ambient = material_ka;
          vec3 specular= material_ks * pow(max(dot(h, v_space_norm),0), material_shininess);
          color = vec4(light.intensity * (specular + diffuse) + ambient, 1);
     }
     else
     {
          color = vec4(material_ka,1);
     }
}