#version 450 core

in VS_OUT
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} vs_out;

out vec4 final_color;

uniform sampler3D tex3D;

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
  vec3 pos = vs_out.world_position.xyz / cube_size;

  if (!within_cube(pos, 0))
    return;

  vec3 voxel_pos = scale_and_bias(pos);
  final_color = textureLod(tex3D, voxel_pos, 0);
}
