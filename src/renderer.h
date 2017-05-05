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
  int cube_res_location;
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

typedef struct
{
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
} point_light_t;

#pragma pack(push, 1)
typedef struct 
{
  glm::vec4 ambient;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::vec4 transmittance;
  glm::vec3 emission;

  float shininess;
  float ior;       // index of refraction
  float dissolve;  // 1 == opaque; 0 == fully transparent

  int illum;

  // PBR extension
  float roughness;            // [0, 1] default 0
  float metallic;             // [0, 1] default 0
  float sheen;                // [0, 1] default 0
  float clearcoat_thickness;  // [0, 1] default 0
  float clearcoat_roughness;  // [0, 1] default 0
  float anisotropy;           // aniso. [0, 1] default 0
  float anisotropy_rotation;  // anisor. [0, 1] default 0

  float pad[2];
} material_data_t;
#pragma pack(pop)

struct Renderer
{
  Renderer(int width, int height);

  ~Renderer(); 

  // Resource 

  model_id_t load_model(const char* filename);
  shader_id_t load_shader(const char* vertex_shader_name,
                          const char* fragment_shader_name,
                          const char* geometry_shader_name = NULL);

  material_id_t add_material(material_data_t& material_data);
  void upload_material_data(material_data_t& material_data, material_id_t material_id);
  void set_model_material(material_id_t material_id, model_id_t model_id);

  glm::vec3 get_model_dimensions(model_id_t model);
  void set_grid_resolution(unsigned int res);
  void set_grid_size(float size);
  
  void set_camera_transform(glm::mat4& lookat, glm::mat4& projection);
  void set_model_transform(model_id_t model_id, glm::mat4 model_matrix);

  material_t& get_material(material_id_t material_id);

  // Render

  void set_rendering_phases(bool direct, bool diffuse, bool specular, bool shadow);
  void set_voxel_view_dir(int dir, float lod);

  void queue_model(model_id_t model_id);
  void queue_point_light(point_light_t& point_light);
  void render();

private:
  void draw_models(const shader_t& shader);
  void filter();
  void upload_lights(const shader_t& shader);
  void upload_camera(const shader_t& shader);
  void voxelize();
  void visualize();
  void bind_material(material_t& material, int location);

  // Lifetime

  std::vector<material_t> m_materials;
  std::vector<model_t> m_models;
  std::vector<draw_obj_t> m_draw_objs;
  std::vector<shader_t> m_shaders;

  GLuint m_camera_ubo;
  camera_data_t m_camera;

  float m_cube_size;
  GLuint m_voxel_grid_tex;
  size_t m_resolution;

  GLuint m_voxel_maps[6];

  int m_viewport_width, m_viewport_height;

  bool m_enable_shadows, m_enable_direct, m_enable_indirect_diffuse, m_enable_indirect_specular;
  int m_view_voxel_dir;
  float m_view_voxel_lod;

  // Per frame

  std::vector<model_t> m_draw_queue;
  std::vector<point_light_t> m_point_lights;

  GLuint m_mipmap_shader;
  shader_id_t m_voxelize_shader;
  shader_id_t m_draw_shader;
};
