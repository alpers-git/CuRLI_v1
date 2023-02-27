#version 450
precision mediump float;
//------------ Structs ------------
struct Light{
     vec3 position;
     vec3 color;
     float intensity;
};

//------------ Variying ------------
layout (location = 3) in vec3 v_space_norm;
layout (location = 4) in vec3 v_space_pos;
layout (location = 5) in vec2 tex_coord;
layout (location = 6) in vec3 w_space_pos;
layout (location = 7) in vec3 w_space_norm;

//------------ Uniforms ------------
uniform mat4 to_view_space; //mv
uniform int light_count;
uniform Light light[8];

uniform struct Material {
     vec3 ka;
     vec3 kd;
     vec3 ks;
     float shininess;
}material;
uniform int shading_mode;//0 = phong-color, 1 = editor mode
uniform int has_texture[5] = {0,0,0,0,0};//[0] = diffuse, [1] = specular, [2] = normal
uniform sampler2D tex_list[5];

uniform samplerCube env_map;
uniform int has_env_map = 0;
uniform vec3 camera_pos;


out vec4 color;

void main() {
     if(shading_mode == 0)//phong shading textures and environment maps
     {
          color = vec4(0,0,0,1);
          vec3 v_space_norm = normalize(v_space_norm);
          for(int i = 0; i < light_count; i++)
          {
               vec3 l =  normalize(light[i].position - v_space_pos);//normalize(l); //light vector
               vec3 h = normalize(l + vec3(0,0,1)); //half vector

               float cos_theta = dot(l, v_space_norm);
               if(cos_theta >= 0)
               {
                    vec3 diffuse =  (has_texture[1]==1 ? (texture(tex_list[1], tex_coord)).xyz :
                                                       material.kd) * max(cos_theta,0);
                    vec3 specular= (has_texture[2]==1 ? (texture(tex_list[2], tex_coord)).xyz :
                                                       material.ks) * pow(max(dot(h, v_space_norm),0), material.shininess);
                    color += vec4(light[i].intensity * normalize(light[i].color) * (specular + diffuse), 1);
               }
          }
          
          color = color + vec4( (has_texture[0]==1 ? (texture(tex_list[0], tex_coord)).xyz :
                                                             material.ka), 1);
          if(has_env_map != 0)
          {
               vec3 env_color = texture(env_map, reflect(-camera_pos+w_space_pos, normalize(w_space_norm))).xyz;
               color = mix(color, vec4(env_color, 1), 0.5);
          }
          color = clamp(color, 0, 1);                                                
     }
     else if(shading_mode == 1)//editor lines and components
     {
          color = vec4(1,1,1,1);
     }
     else if(shading_mode == 2)//skybox background
     {
          color = texture(env_map, normalize(v_space_pos));
     }
     else
     {
          color = vec4(0,0,0,1);
     }
}