#version 450 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} vs_out[];

out GS_OUT
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} gs_out;


void main()
{
  // Get triangle normal
  const vec3 p1 = vs_out[1].world_position.xyz - vs_out[0].world_position.xyz;
  const vec3 p2 = vs_out[2].world_position.xyz - vs_out[0].world_position.xyz;
  const vec3 p = abs(normalize(cross(p1, p2)));

  // For each vertex emit a projected vertex based on the axis with the
  // largest projection
  for (uint i = 0; i < 3; i++)
  {
    gs_out.world_position = vs_out[i].world_position;
    gs_out.normal = vs_out[i].normal;
    gs_out.uv = vs_out[i].uv;

    if (p.z > p.x && p.z > p.y)
    {
      gl_Position = vec4(gs_out.world_position.x, gs_out.world_position.y, 0, 1);
    }
    else if (p.x > p.y && p.x > p.z)
    {
      gl_Position = vec4(gs_out.world_position.y, gs_out.world_position.z, 0, 1);
    }
    else
    {
      gl_Position = vec4(gs_out.world_position.x, gs_out.world_position.z, 0, 1);
    }
    EmitVertex();
  }
  EndPrimitive();
}
