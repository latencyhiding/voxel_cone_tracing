#version 450 core

in GS_OUT 
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} gs_out;

layout (std140, binding = 1) uniform material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  vec3 transmittance;
  vec3 emission;

  float shininess;
  float ior;
  float dissolve;

  int illum;

  float roughness;
  float metallic;
  float sheen;
  float clearcoat_thickness;
  float clearcoat_roughness;
  float anisotropy;
  float anisotropy_rotation;
};

struct point_light
{
  vec3 position;
  vec3 color;
};

#define MAX_POINT_LIGHTS 10
uniform point_light point_lights[MAX_POINT_LIGHTS];
uniform int point_light_count;

uniform layout (RGBA8) image3D tex3D;

vec3 scale_and_bias(const vec3 p)
{
  return 0.5f * p + vec3(0.5f);
}

bool within_cube(const vec3 p, float error)
{
  return abs(p.x) < 1 + error && abs(p.y) < 1 + error && abs(p.z) < 1 + error;
}

float attenuate(float k, float l, float q, float d)
{
  return 1.0f / (k + l * d + q * d * d);
}

uniform float cube_size;

void main()
{
  vec3 pos = gs_out.world_position.xyz;

  //if (!within_cube(pos, 0));
  //  return;

  vec3 color = vec3(0.0f);

  // Calculate lighting
  int num_lights = min(point_light_count, MAX_POINT_LIGHTS);
  for (int i = 0; i < num_lights; i++)
  {
    const vec3 light_pos = point_lights[i].position / cube_size;
    const vec3 dir = normalize(light_pos - gs_out.world_position.xyz);
    const float d = distance(light_pos, gs_out.world_position.xyz);
    const float a = attenuate(1, 0, 1, d);

    float cos_surf = max(dot(normalize(gs_out.normal), dir), 0.0f);

    // Temperorary measure as for some reason the winding is wrong
    
    color += cos_surf * a * point_lights[i].color;
  }

  color *= (diffuse + specular);
  color += emission;

  vec4 final_color = vec4(color, 1);
  
  // TODO: Transmittance filter and index of refraction
  final_color *= pow(dissolve, 4);

  // Output to 3D texture
  ivec3 dim = imageSize(tex3D);
  ivec3 voxel_pos = ivec3(dim * scale_and_bias(pos));

  imageStore(tex3D, voxel_pos, final_color);
}
