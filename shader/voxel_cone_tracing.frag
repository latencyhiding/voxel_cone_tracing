#version 450 core

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

in VS_OUT
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} vs_out;

out vec4 final_color;

uniform sampler3D tex3D;

uniform float cube_size;

uniform int cube_res;

uniform vec3 camera_position;

float voxel_size = 1.0f / cube_res;

struct point_light
{
  vec3 position;
  vec3 color;
};

#define MAX_POINT_LIGHTS 10
uniform point_light point_lights[MAX_POINT_LIGHTS];
uniform int point_light_count;

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

vec3 trace_cone(vec3 origin, vec3 dir, float aperture)
{
  dir = normalize(dir);

  vec4 sample_color = vec4(0.0);
  float dist = 3 * voxel_size;
  float diam = dist * aperture;
  vec3 sample_position = dir * dist + origin;

  float max_dist = sqrt(3);

  // Step until alpha > 1 or out of bounds
  while (sample_color.a < 1.0 && dist < max_dist)// && length(sample_position) <= 1)
  {
    // Choose mip level based on the diameter of the cone
    float mip = max(log2(diam * cube_res), 0);

    vec4 mip_sample = textureLod(tex3D, sample_position, mip);

    // Blend mip sample with current sample color
    sample_color += (1 - sample_color.a) * mip_sample;
    
    float step_size = max(diam, voxel_size);

    dist += step_size;
    diam = dist * aperture;
    sample_position = dir * dist + origin;
  }

  return sample_color.xyz;
}

float trace_shadow(vec3 origin, vec3 dir, float aperture, float target_dist)
{
  dir = normalize(dir);

  float alpha = 0.0;
  float dist = 3 * voxel_size;
  float diam = dist * aperture;
  vec3 sample_position = dir * dist + origin;

  // Step until alpha > 1 or out of bounds
  while (alpha < 1.0 && dist < target_dist)
  {
    // Choose mip level based on the diameter of the cone
    float mip = max(log2(diam * cube_res), 0);

    vec4 mip_sample = textureLod(tex3D, sample_position, mip);

    // Blend mip sample with current sample color
    alpha += (1 - alpha) * mip_sample.a;
    
    float step_size = max(diam, voxel_size);

    dist += step_size;
    diam = dist * aperture;
    sample_position = dir * dist + origin;
  }

  return alpha;
}

vec3 tangent(vec3 n)
{
  vec3 t1 = cross(n, vec3(0, 0, 1));
  vec3 t2 = cross(n, vec3(0, 1, 0));
  if (length(t1) > length(t2))
    return normalize(t1);
  else
    return normalize(t2);
}

vec3 trace_diffuse(vec3 origin, vec3 normal)
{
  const float angle_mix = 0.5f;
  const float aperture = sqrt(2) / 2;
  vec3 result_diffuse = vec3(0.0); 
  
  const vec3 o1 = normalize(tangent(normal));
  const vec3 o2 = normalize(cross(o1, normal));

  const vec3 c1 = 0.5f * (o1 + o2);
  const vec3 c2 = 0.5f * (o1 - o2);

  // Normal direction
  result_diffuse += trace_cone(origin, normal, aperture);

  // 4 side cones
  result_diffuse += trace_cone(origin, mix(normal, o1, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, -o1, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, o2, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, -o2, angle_mix), aperture);

  // 4 corners
  result_diffuse += trace_cone(origin, mix(normal, c1, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, -c1, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, c2, angle_mix), aperture);
  result_diffuse += trace_cone(origin, mix(normal, -c2, angle_mix), aperture);

  return result_diffuse / 9.0f;
}

vec3 direct_light(vec3 pos, vec3 view_dir)
{
  vec3 result = vec3(0.0f);

  for (uint i = 0; i < min(point_light_count, MAX_POINT_LIGHTS); i++)
  {
    point_light light = point_lights[i];

    vec3 light_pos = scale_and_bias(light.position / cube_size);
    vec3 light_dir = light_pos - pos;
    float d = length(light_dir);
    light_dir = normalize(light_dir);
    float cos_surf = max(dot(vs_out.normal, light_dir), 0.0f);
    vec3 light_color = attenuate(1, 0, 1, d) * cos_surf * light.color;

    // Shadow
    float shadow_level = max(0, 1 - trace_shadow(pos, light_dir, 0.08, d));

    // Blinn-Phong
    float lambertian = max(dot(light_dir, vs_out.normal), 0);
    lambertian = min(shadow_level, lambertian);

    float specular_coeff = 0;

    vec3 half_vec = normalize(light_dir + view_dir);
    float specular_angle = max(dot(half_vec, vs_out.normal), 0);
    specular_angle = min(shadow_level, specular_angle);

    specular_coeff = pow(specular_angle, shininess);

    result += (lambertian * diffuse + specular_coeff * specular) * light_color;
  }

  return result;
}

vec3 trace_specular(vec3 pos, vec3 normal, vec3 view_dir)
{
  vec3 specular_dir = -reflect(view_dir, normal);
  return trace_cone(pos, specular_dir, roughness);
}

void main()
{
  vec3 pos = scale_and_bias(vs_out.world_position.xyz / cube_size);
  vec3 view_dir = normalize(vs_out.world_position.xyz - camera_position);

  if (!within_cube(pos, 0))
    return;

  vec3 final_diffuse = trace_diffuse(pos, vs_out.normal);
  final_diffuse += emission * diffuse;
  vec3 final_direct = direct_light(pos, view_dir);
  vec3 final_specular = trace_specular(pos, vs_out.normal, view_dir);

  final_color = vec4(diffuse * (final_specular + final_diffuse + final_direct), 1); 
}
