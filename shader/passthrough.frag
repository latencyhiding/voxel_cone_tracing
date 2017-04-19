#version 430

out vec4 final_color;

in VS_OUT 
{
  vec4 position;
  vec3 normal;
  vec2 uv;
} fs_in;

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

void main()
{
  final_color = vec4(ambient, 1.0);
}
