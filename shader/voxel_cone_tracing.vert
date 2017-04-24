#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (binding = 0) uniform camera
{
  mat4 projection;
  mat4 view;
};

out vs_out
{
  vec4 world_position;
  vec3 normal;
  vec2 uv;
} vs_out;

uniform mat4 model;

void main()
{
  vs_out.position = model * vec4(position, 1.0);
  gl_Position = projection * view * vs_out.position;
  vs_out.normal = normal;
  vs_out.uv = uv;
}
