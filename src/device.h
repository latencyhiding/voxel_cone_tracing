#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#define MAX_BOUND_TEXTURES 32
#define MAX_BOUND_UBOS 32

typedef size_t pipeline_id_t;
typedef size_t vertex_buffer_t;

typedef struct
{
  GLuint vao;
  GLuint shader_program;

  GLuint textures[MAX_BOUND_TEXTURES];
  size_t num_textures;

  GLuint ubos[MAX_BOUND_UBOS];
  size_t num_ubos;
} pipeline_state_t;

typedef struct
{

    
} attrib_description_t;

class Device
{
  public:
    
  private:
    std::vector<pipeline_state_t> m_pipeline_states;
};
