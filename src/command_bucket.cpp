#include "command_bucket.h"
#include <glm/gtc/type_ptr.hpp>

CommandBucket::CommandBucket(Renderer& renderer)
  : m_renderer(renderer)
{
}

void CommandBucket::push_command(draw_obj_t& draw_obj, shader_id_t shader_id, uniform_t* uniforms, size_t num_uniforms)
{
  
}

void CommandBucket::submit()
{
}

void CommandBucket::clear()
{
}

void CommandBucket::upload_uniforms(buffer_range_t& uniforms_range)
{
  for (size_t i = uniforms_range.start; i < uniforms_range.start + uniforms_range.size; i++)
  {
    uniform_t& uniform = m_uniforms[i];
    switch (uniform.type)
    {
      case uniform_t::MAT4:
        glUniformMatrix4fv(uniform.location, 1, false, glm::value_ptr(uniform.mat4));
        break;
      case uniform_t::VEC3:
        glUniform3fv(uniform.location, 1, glm::value_ptr(uniform.vec3));
        break;
      default:
        break;
    };
  }
}

void CommandBucket::bind_material(material_t& material, int location)
{
  glBindBufferBase(GL_UNIFORM_BUFFER, location, material.ubo);

  for (size_t i = 0; i < material.num_textures; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, material.textures[i]);
  }
}

