// TODO: Eliminate std
// TODO: Make backend agnostic
// TODO: Buckets accept commands rather than draw_objs
// TODO: Handles should actually do something; currently they're just indices
// TODO: Move shader loading to the device

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "shared.h"

#include <glm/glm.hpp>

#define MAX_TEXTURE_BINDINGS 32

typedef GLuint shader_program_t;

#define INVALID_ID (~0)
typedef size_t model_id_t;
typedef size_t shader_id_t;

// Uniforms are copied to a host-memory buffer which is then uploaded as a
// whole to the GPU on submit time.
// Then on each individual draw call, it binds a range in the ubo
typedef struct
{
  size_t start;
  size_t size;  
} buffer_range_t;

// Maps to a uniform buffer and a max MAX_TEXTURE_BINDINGS # of textures
typedef struct
{
  GLuint ubo;
  GLuint textures[MAX_TEXTURE_BINDINGS];
  size_t num_textures;
} material_t;

// Shader object that contains some binding info 
typedef struct
{
  shader_program_t program;
  int material_location;
  int camera_location;
} shader_t;

// Maps to a vbo, a vao, an ebo, a range each for uniforms and indices/vertices
typedef struct
{
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  buffer_range_t range;
  
  GLenum draw_type;
  
  shader_id_t shader_id;
  size_t material_index;

  buffer_range_t uniform_range;
} draw_obj_t;

// Group of draw_objs that share buffer resources
typedef struct
{
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  size_t draw_objs_start_index;
  size_t num_draw_objs;
} model_t;

typedef struct
{
  enum
  {
    MAT4,
    VEC3
  } type;

  union
  {
    glm::mat4 mat4;
    glm::vec3 vec3;
  };

  int location;
} uniform_t;

typedef struct
{
  glm::mat4 projection;
  glm::mat4 view;
} camera_data_t;

struct renderer
{
  renderer();

  ~renderer(); 

  // Resource 

  model_id_t load_model(const char* filename);
  shader_id_t load_shader(const char* vertex_shader_name,
                          const char* fragment_shader_name,
                          const char* geometry_shader_name = NULL);
  int shader_get_uniform_location(shader_id_t, const char* name);

  void set_camera_transform(glm::mat4& lookat, glm::mat4& projection);

  // Render

  void queue_model(model_id_t model_id, shader_id_t shader_id, uniform_t* uniforms, size_t num_uniforms);
  void render();

private:
  void upload_uniforms(buffer_range_t& uniforms_range);
  void bind_material(material_t& material, int location);

  // Lifetime

  std::vector<material_t> m_materials;
  std::vector<model_t> m_models;
  std::vector<draw_obj_t> m_draw_objs;
  std::vector<shader_t> m_shaders;

  GLuint m_camera_ubo;
  camera_data_t m_camera;

  // Per frame

  std::vector<uniform_t> m_uniforms;

  // temp, replace this with command buffer
  std::vector<draw_obj_t> m_draw_queue;
};
