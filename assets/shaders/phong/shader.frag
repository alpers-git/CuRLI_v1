#version 450
precision mediump float;
//------------ Structs ------------
struct Light{
     vec3 position;
     vec3 color;
     float intensity;
};

//------------ Variying ------------
layout (location = 2) in vec3 v_space_norm;
layout (location = 3) in vec3 v_space_pos;

//------------ Uniforms ------------
layout(location = 1) uniform mat4 to_view_space; //mv
uniform int light_count;
uniform Light light[8];


// layout(location = 5) uniform vec3 material_ka;
// layout(location = 6) uniform vec3 material_kd;
// layout(location = 7) uniform vec3 material_ks;
// layout(location = 8) uniform float material_shininess;
uniform struct Material {
     vec3 ka;
     vec3 kd;
     vec3 ks;
     float shininess;
}material;

// layout(location = 5) uniform Material material;

out vec4 color;

void main() {
     color = vec4(0,0,0,1);
     vec3 v_space_norm = normalize(v_space_norm);
     for(int i = 0; i < light_count; i++)
     {
          vec3 l =  normalize(light[i].position - v_space_pos);//normalize(l); //light vector
          vec3 h = normalize(l + vec3(0,0,1)); //half vector

          float cos_theta = dot(l, v_space_norm);
          if(cos_theta >= 0)
          {
               vec3 diffuse = material.kd * max(cos_theta,0);
               vec3 specular= material.ks * pow(max(dot(h, v_space_norm),0), material.shininess);
               color += vec4(light[i].intensity * normalize(light[i].color) * (specular + diffuse), 1);
          }
     }

     color = clamp(color + vec4(material.ka,1),0,1);
}