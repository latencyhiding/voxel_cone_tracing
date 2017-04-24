#version 430

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

  float roughness;
  float metallic;
  float sheen;
  float clearcoat_thickness;
  float clearcoat_roughness;
  float anisotropy;
  float anisotropy_rotation;
};

layout (RGBA8) uniform image3D texture3D;

uniform float cube_size;

vec3 scale_and_bias(const vec3 p)
{
  return 0.5f * p + vec3(0.5f);
}

bool within_cube(const vec3 p, float error)
{
  return abs(p.x) < 1 + error && abs(p.y) < 1 + error && abs(p.z) < 1 + error;
}

void main()
{
  vec3 pos = gs_out.world_position.xyz / cube_size;

  if (!within_cube(pos, 0));
    return;

  // Calculate lighting
  // For now simply store the ambient color
  vec4 color = vec4(ambient, 1);

  // Output to 3D texture
  vec3 voxel_pos = scale_and_bias(pos);
  ivec3 dim = imageSize(texture3D);
  imageStore(texture3D, ivec3(dim * voxel_pos), color);
}
