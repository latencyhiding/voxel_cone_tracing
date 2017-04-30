#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;


layout (binding = 0) uniform camera
{
  mat4 projection;
  mat4 view;
};

uniform mat4 model;

out VS_OUT
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} vs_out;

uniform float cube_size;

void main()
{
  vs_out.world_position = model * vec4(position / cube_size, 1.0);
  gl_Position = projection * view * vs_out.world_position;
  vs_out.normal = normal;
  vs_out.uv = uv;
}

