#version 450
precision highp float;

const vec2 poissonDisk[4] = vec2[](
  vec2( -0.94201624, -0.39906216 ),
  vec2( 0.94558609, -0.76890725 ),
  vec2( -0.094184101, -0.92938870 ),
  vec2( 0.34495938, 0.29387760 )
);

//------------ Structs ------------
struct PLight{
    vec3 position;
    vec3 color;
    float intensity;
};
vec3 illuminationAt(in PLight light, in vec3 pos, inout vec3 l)
{
     l = (light.position - pos);
     float l_length = length(l);
     l = normalize(l);
     return light.intensity * normalize(light.color) / (0.001*l_length * l_length + 0.001*l_length);
}

struct DLight{
    vec3 direction;
    vec3 color;
    float intensity;
    int casting_shadows;
    mat4 to_light_view_space;
};

vec3 illuminationAt(in DLight light, in vec3 pos, in sampler2DShadow shadow_map, in vec3 w_space_pos, inout vec3 l)
{
     l = -normalize(light.direction);
     float intensity = light.intensity;

     if(light.casting_shadows == 1)
     {
          vec4 lv_space_pos = light.to_light_view_space * vec4(w_space_pos, 1.0);
          float shadow = 0;
          for (int i=0;i<4;i++){
                if(textureProj(shadow_map, lv_space_pos + vec4(poissonDisk[i]/700, 0, 0)) < lv_space_pos.z)
                    shadow += 0.25;
          }
          intensity -= shadow;
     }
     return intensity * normalize(light.color);
}
struct SLight{
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float cutoff;
    int casting_shadows;
    mat4 to_light_view_space;
};
vec3 illuminationAt(in SLight light, in vec3 pos, in sampler2DShadow shadow_map, in vec3 w_space_pos, inout vec3 l)
{
     l = light.position - pos;
     float l_length = length(light.position - pos);
     l = normalize(l);
     const float spot_factor = dot(l, -normalize(light.direction));
     float intensity = 0;

     if (spot_factor > light.cutoff)
     {
          intensity = light.intensity * 
                    (1.0 - (1.0 - spot_factor) * 1.0/(1.0 - light.cutoff));
          if(light.casting_shadows == 1)
          {
               vec4 lv_space_pos = light.to_light_view_space * vec4(w_space_pos, 1.0);
               float shadow = 0;
               for (int i=0;i<4;i++){
                    if(textureProj(shadow_map, lv_space_pos + vec4(poissonDisk[i]/700, 0, 0)) < 1.0)
                         shadow += 0.25;
               }
               //intensity -= shadow;
               intensity *= textureProj(shadow_map, lv_space_pos);
               intensity = clamp(intensity, 0.0, 1.0); 
          }
     }


     return intensity * normalize(light.color) / (0.0005*l_length * l_length + 0.0001*l_length);
}


//------------ Variying ------------
layout (location = 3) in vec3 v_space_norm;
layout (location = 4) in vec3 v_space_pos;
layout (location = 5) in vec2 tex_coord;
layout (location = 6) in vec3 w_space_pos;
layout (location = 7) in vec3 w_space_norm;
layout (location = 8) in vec4 lv_space_pos;

//------------ Uniforms ------------
uniform mat4 to_view_space; //mv

uniform int p_light_count;
uniform PLight p_lights[5];
uniform int d_light_count;
uniform DLight d_lights[5];
uniform sampler2DShadow d_shadow_maps[5];
uniform int s_light_count;
uniform SLight s_lights[5];
uniform sampler2DShadow s_shadow_maps[5];

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
uniform int mirror_reflection = 0;

out vec4 color;

void main() {
     if(shading_mode == 0)//phong shading textures and environment maps
     {
          color = vec4(0,0,0,1);
          float shadow = 0;
          vec3 v_space_norm = normalize(v_space_norm);
          for(int i = 0; i < p_light_count + d_light_count + s_light_count; i++)
          {
               vec3 l = vec3(0,0,0);
               vec3 illumination = vec3(0,0,0); // light color * intensity
               //vec3 light_color = vec3(0,0,0);

               const int d_light_index = i - p_light_count;
               const int s_light_index = i - p_light_count - d_light_count;
               
               if(i < p_light_count) //point light soures
               {
                    illumination = illuminationAt(p_lights[i], v_space_pos, l);
               }
               else if(d_light_index < d_light_count) //directional light sources
               {
                    illumination = illuminationAt(d_lights[d_light_index], v_space_pos, d_shadow_maps[d_light_index], w_space_pos, l);
               }
               else
               {
                    illumination = illuminationAt(s_lights[s_light_index], v_space_pos, s_shadow_maps[s_light_index], w_space_pos, l);
                    //light_color = s_lights[s_light_index].color;
                    
               }

               vec3 h = normalize(l + vec3(0,0,1)); //half vector

               float cos_theta = dot(l, v_space_norm);
               if(cos_theta >= 0) //getting light from the front side of the surface
               {    
                    //Sample either texture or material color
                    vec3 diffuse =  (has_texture[1]==1 ? (texture(tex_list[1], tex_coord)).xyz :
                                                       material.kd) * max(cos_theta,0);
                    vec3 specular= (has_texture[2]==1 ? (texture(tex_list[2], tex_coord)).xyz :
                                                       material.ks) * pow(max(dot(h, v_space_norm),0), material.shininess);
                    color += vec4(illumination * (specular + diffuse), 1);
               }
          }
          
          color = color + 0.2 * vec4( (has_texture[0]==1 ? (texture(tex_list[0], tex_coord)).xyz :
                                                            material.ka), 1);
          if(has_env_map != 0)//sample environment map if it exists
          {
               vec3 env_color = texture(env_map, reflect(-camera_pos+w_space_pos, normalize(w_space_norm))).xyz;
               color = mix(color, vec4(env_color, 1), 0.50); //mix it with material color
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
     else//error
     {
          color = vec4(1,0,1,1);
     }
}