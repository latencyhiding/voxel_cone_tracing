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

layout (binding = 2) uniform sampler3D tex3D[6];

uniform float cube_size;

uniform int cube_res;

uniform vec3 camera_position;

float voxel_size = 1.0f / cube_res;

struct point_light
{
  vec3 position;
  vec3 color;
  float intensity;
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

uvec3 face_indices(vec3 dir)
{
  uvec3 ret;
  ret.x = (dir.x < 0.0) ? 0 : 1;
  ret.y = (dir.y < 0.0) ? 2 : 3;
  ret.z = (dir.z < 0.0) ? 4 : 5;
  return ret;
}

vec4 sample_voxel(vec3 pos, vec3 dir, uvec3 indices, float lod)
{
  dir = abs(dir);
  return dir.x * textureLod(tex3D[indices.x], pos, lod) +
         dir.y * textureLod(tex3D[indices.y], pos, lod) +
         dir.z * textureLod(tex3D[indices.z], pos, lod);
}

vec4 trace_cone(vec3 origin, vec3 dir, float aperture, float max_dist)
{
  dir = normalize(dir);

  uvec3 indices = face_indices(dir);

  vec4 sample_color = vec4(0.0);
  float dist = 3 * voxel_size;
  float diam = dist * aperture;
  vec3 sample_position = dir * dist + origin;

  // Step until alpha > 1 or out of bounds
  while (sample_color.a < 1.0 && dist < max_dist)
  {
    // Choose mip level based on the diameter of the cone
    float mip = max(log2(diam * cube_res), 0);

    vec4 mip_sample = sample_voxel(sample_position, dir, indices, mip);

    // Blend mip sample with current sample color
    sample_color += (1 - sample_color.a) * mip_sample;
    
    float step_size = max(diam / 2, voxel_size);

    dist += step_size;

    diam = dist * aperture;
    sample_position = dir * dist + origin;
  }

  return sample_color;
}

float trace_shadow(vec3 origin, vec3 dir, float aperture, float target_dist)
{
  return trace_cone(origin, dir, aperture, target_dist).a;
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

#define TAN_22_5 0.55785173935
// sqrt(3)
#define MAX_DISTANCE 1.73205080757

vec3 trace_diffuse(vec3 origin, vec3 normal)
{
  const float angle_mix = 0.5f;
  const float aperture = TAN_22_5;
  vec4 result_diffuse = vec4(0.0); 
  
  const vec3 o1 = normalize(tangent(normal));
  const vec3 o2 = normalize(cross(o1, normal));

  const vec3 c1 = 0.5f * (o1 + o2);
  const vec3 c2 = 0.5f * (o1 - o2);

  // Normal direction
  result_diffuse += trace_cone(origin, normal, aperture, MAX_DISTANCE);

  // 4 side cones
  result_diffuse += trace_cone(origin, mix(normal, o1, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, -o1, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, o2, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, -o2, angle_mix), aperture, MAX_DISTANCE);

  // 4 corners
  result_diffuse += trace_cone(origin, mix(normal, c1, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, -c1, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, c2, angle_mix), aperture, MAX_DISTANCE);
  result_diffuse += trace_cone(origin, mix(normal, -c2, angle_mix), aperture, MAX_DISTANCE);

  return result_diffuse.xyz / 9.0f;
}

uniform bool enable_diffuse;
uniform bool enable_specular;
uniform bool enable_shadow;
uniform bool enable_direct;

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
    vec3 light_color = attenuate(1, 0, 1, d) * cos_surf * light.color * light.intensity;

    // Shadow
    float shadow_level = 1.0;
    if (enable_shadow)
      shadow_level = max(0, 1 - trace_shadow(pos, light_dir, 0.1, d));

    // Blinn-Phong
    float lambertian = max(dot(light_dir, vs_out.normal), 0);

    // Refraction
    float refract_angle = 0.0;
    if (dissolve <= 0.1)
    {
      vec3 refraction = refract(view_dir, vs_out.normal, 1.0 / ior);
      refract_angle = max((1 - dissolve) * dot(refraction, light_dir), 0);
    }

    float specular_coeff = 0;

    vec3 half_vec = normalize(light_dir + view_dir);
    float specular_angle = max(dot(half_vec, vs_out.normal), 0);
    specular_angle = max(specular_angle, refract_angle);

    specular_coeff = pow(specular_angle, shininess);

    result += (shadow_level + 0.04) * (lambertian * diffuse + specular_coeff * specular) * light_color;
  }

  return result + clamp(emission, 0, 1);
}

float spec_to_roughness = sqrt(2.0f / (shininess + 2.0f));

#define PI 3.14159265f
#define HALF_PI 1.57079f
#define MIN_SPECULAR_APERTURE 0.0174533f
#define MAX_SPECULAR_APERTURE PI

float specular_aperture = clamp(tan(HALF_PI * (spec_to_roughness)), MIN_SPECULAR_APERTURE, MAX_SPECULAR_APERTURE);

vec3 trace_specular(vec3 pos, vec3 normal, vec3 view_dir)
{
  vec3 specular_dir = normalize(reflect(-view_dir, normal));

  // Clamp to 1 grad and pi, exponent is angle of cone in radians
  return trace_cone(pos, specular_dir, specular_aperture, MAX_DISTANCE).xyz;
}

vec3 trace_refraction(vec3 pos, vec3 normal, vec3 view_dir)
{
  const vec3 refraction = refract(view_dir, normal, 1.0 / ior);
  return transmittance * trace_cone(pos, refraction, specular_aperture, MAX_DISTANCE).xyz;
}

uniform int view_voxel_dir;
uniform float view_voxel_lod;

void main()
{
  vec3 pos = scale_and_bias(vs_out.world_position.xyz / cube_size);

  if (!within_cube(pos, 0))
    return;

  if (view_voxel_dir < 7)
  {
    final_color = textureLod(tex3D[view_voxel_dir], pos, view_voxel_lod);
    return;
  }

  vec3 view_dir = normalize(vs_out.world_position.xyz - camera_position);

  vec3 final_diffuse = vec3(0.0);
  vec3 final_direct = vec3(0.0);
  vec3 final_specular = vec3(0.0);

  if (enable_diffuse)
    final_diffuse = diffuse * trace_diffuse(pos, vs_out.normal);
  if (enable_direct)
    final_direct = direct_light(pos, view_dir);
  if (enable_specular)
    final_specular = specular * trace_specular(pos, vs_out.normal, view_dir);

  final_color = vec4(final_specular + final_diffuse + final_direct, 1);
  if ((illum == 4 || illum == 6 || illum == 7 || illum == 9) && enable_specular)
    final_color = vec4(mix(trace_refraction(pos, vs_out.normal, view_dir), final_color.rgb, dissolve), 1);
}
