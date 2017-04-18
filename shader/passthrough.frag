#version 430

out vec4 final_color;

in VS_OUT 
{
  vec4 position;
  vec3 normal;
  vec2 uv;
} fs_in;

void main()
{
  final_color = vec4(1.0, 1.0, 1.0, 1.0); 
}
