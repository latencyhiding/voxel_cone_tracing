#version 430

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (binding = 0) uniform camera
{
  mat4 projection;
  mat4 view;
};

out VS_OUT
{
  vec4 position;
  vec3 normal;
  vec2 uv;
} vs_out;

void main()
{
  gl_Position = projection * view * vec4(position, 1.0);
  vs_out.position = vec4(position, 1.0);
  vs_out.normal = normal;
  vs_out.uv = uv;
}
