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
#include "camera.h"

#include <glm/glm.hpp>

#define MAX_TEXTURE_BINDINGS 32

typedef GLuint shader_program_t;

#define INVALID_ID (~0)
typedef size_t model_id_t;
typedef size_t shader_id_t;
typedef size_t material_id_t;

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
  int texture_3D_location;
  int cube_size_location;
  int model_location;
} shader_t;

// Maps to a vbo, a vao, an ebo, a range each for uniforms and indices/vertices
typedef struct
{
  buffer_range_t range;
  
  GLenum draw_type;
  
  material_id_t material_id;
} draw_obj_t;

// Group of draw_objs that share buffer resources
typedef struct
{
  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  buffer_range_t draw_obj_range;

  shader_id_t shader_id;

  glm::vec3 dimensions;
  glm::mat4 model_matrix;
} model_t;

typedef struct
{
  glm::mat4 projection;
  glm::mat4 view;
} camera_data_t;

struct Renderer
{
  Renderer();

  ~Renderer(); 

  // Resource 

  model_id_t load_model(const char* filename);
  shader_id_t load_shader(const char* vertex_shader_name,
                          const char* fragment_shader_name,
                          const char* geometry_shader_name = NULL);
  glm::vec3 get_model_dimensions(model_id_t model);
  void set_grid_resolution(unsigned int res);
  void set_grid_size(float size);
  
  void set_camera_transform(glm::mat4& lookat, glm::mat4& projection);
  void set_model_transform(model_id_t model_id, glm::mat4 model_matrix);

  material_t& get_material(material_id_t material_id);

  // Render

  void queue_model(model_id_t model_id);
  void render();

private:
  void bind_material(material_t& material, int location);

  void render_queue(std::vector<model_t>& draw_queue);

  // Lifetime

  std::vector<material_t> m_materials;
  std::vector<model_t> m_models;
  std::vector<draw_obj_t> m_draw_objs;
  std::vector<shader_t> m_shaders;

  GLuint m_camera_ubo;
  camera_data_t m_camera;

  float m_cube_size;
  GLuint m_voxel_grid_tex;

  // Per frame

  std::vector<model_t> m_voxel_queue;
  std::vector<model_t> m_draw_queue;

  shader_id_t m_voxelize_shader;
  shader_id_t m_draw_shader;
};
