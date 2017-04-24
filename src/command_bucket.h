#pragma once

#include "renderer.h"

struct CommandBucket 
{
  public:
    CommandBucket(Renderer& renderer);
    void push_command(draw_obj_t& draw_obj, shader_id_t shader_id, uniform_t* uniforms, size_t num_uniforms);
    void submit();
    void clear(); 
  private:
    void upload_uniforms(buffer_range_t& uniforms_range);
    void bind_material(material_t& material, int location);

    std::vector<draw_obj_t> m_draw_queue;
    std::vector<uniform_t> m_uniforms;

    Renderer& m_renderer;
};

