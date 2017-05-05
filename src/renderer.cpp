// TODO: Split into pass class
// TODO: Finish shader implementation
// TODO: Write your own damn object loader
// TODO: Support textures
// TODO: Materials should not just bind 2D textures
// TODO: Calculate normals if not provided

#include <tinyobjloader/tiny_obj_loader.h>
#include <unordered_map>

#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_access.hpp> 

#include <gl_utils/gl_helpers.h>

#include "texture_3d.h"

#pragma pack(push, 1)
struct vert_data_t
{
  glm::vec3 pos;
  glm::vec3 norm;
  glm::vec2 tex;

  bool operator==(const vert_data_t& other) const
  {
    return pos == other.pos && norm == other.norm && tex == other.tex;
  }
};
#pragma pack(pop)

namespace std
{
  template<> struct hash<vert_data_t>
  {
    size_t operator()(vert_data_t const& vertex) const
    {
      return ((hash<glm::vec3>()(vertex.pos) ^
              (hash<glm::vec3>()(vertex.norm) << 1)) >> 1) ^ 
              (hash<glm::vec2>()(vertex.tex) << 1);
    }
  };
}
void create_material(const tinyobj::material_t& material, material_t* result)
{
  material_data_t mat_data;

  for (int i = 0; i < 3; i++)
  {
    mat_data.ambient[i] = material.ambient[i];
    mat_data.diffuse[i] = material.diffuse[i];
    mat_data.specular[i] = material.specular[i];
    mat_data.transmittance[i] = material.transmittance[i];
    mat_data.emission[i] = material.emission[i];
  }
  
  mat_data.shininess = material.shininess;
  mat_data.ior = material.ior;
  mat_data.dissolve = material.dissolve;
  mat_data.illum = material.illum;
  mat_data.roughness = material.roughness;
  mat_data.metallic = material.metallic;
  mat_data.sheen = material.sheen;
  mat_data.clearcoat_thickness = material.clearcoat_thickness;
  mat_data.clearcoat_roughness = material.clearcoat_roughness;
  mat_data.anisotropy = material.anisotropy;
  mat_data.anisotropy_rotation = material.anisotropy_rotation;

  glGenBuffers(1, &result->ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, result->ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(material_data_t), &mat_data, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  //temp
  result->num_textures = 0;
}

#define DEFAULT_MATERIAL_INDEX 0

static void fill_default_mat_data(material_data_t& mat)
{
  mat.ambient[0] = 1;
  mat.ambient[1] = 0;
  mat.ambient[2] = 1;
  mat.diffuse[0] = 1;
  mat.diffuse[1] = 0;
  mat.diffuse[2] = 1;

  mat.shininess = 1;
  mat.ior = 1;
  mat.dissolve = 1;
  mat.illum = 0;
}

Renderer::Renderer(int width, int height)
  : m_viewport_width(width)
  , m_viewport_height(height)
  , m_enable_shadows(true)
  , m_enable_direct(true)
  , m_enable_indirect_diffuse(true)
  , m_enable_indirect_specular(true)
  , m_view_voxel_dir(0)
  , m_view_voxel_lod(0)
{
  // Setup camera ubo
  glGenBuffers(1, &m_camera_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(camera_data_t), &m_camera, GL_DYNAMIC_DRAW);

  // Push default material
  material_data_t default_material_data;

  fill_default_mat_data(default_material_data);

  material_t default_material;
  default_material.num_textures = 0;

  glGenBuffers(1, &default_material.ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, default_material.ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(material_t), &default_material, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  m_materials.push_back(default_material);

  // Load shaders
  m_draw_shader = load_shader("shader/voxel_cone_tracing.vert",
                              "shader/voxel_cone_tracing.frag");

  m_voxelize_shader = load_shader("shader/voxelize.vert",
                                  "shader/voxelize.frag",
                                  "shader/voxelize.geom");

  GLuint mipmap_shader = load_shader_source("shader/mipmap.comp", GL_COMPUTE_SHADER);
  m_mipmap_shader = link_shader_program(&mipmap_shader, 1);
  detach_shaders(m_mipmap_shader, &mipmap_shader, 1);
  destroy_shader(mipmap_shader);

  for (int i = 0; i < 6; i++)
    m_voxel_maps[i] = 0;

  set_grid_resolution(128);
}

Renderer::~Renderer()
{
  for (const auto& material : m_materials)
    glDeleteBuffers(1, &material.ubo);
  for (const auto& model : m_models)
  {
    glDeleteVertexArrays(1, &model.vao);
    glDeleteBuffers(1, &model.vbo);
    glDeleteBuffers(1, &model.ebo);
  }
  for (const auto& shader : m_shaders)
    destroy_program(shader.program);
  glDeleteBuffers(1, &m_camera_ubo);
  destroy_program(m_mipmap_shader);

  glDeleteTextures(6, m_voxel_maps);
}

void Renderer::set_camera_transform(glm::mat4& lookat, glm::mat4& projection)
{
  m_camera.view = lookat;
  m_camera.projection = projection;
}

glm::vec3 Renderer::get_model_dimensions(model_id_t model)
{
  return m_models[model].dimensions;
}

void Renderer::set_grid_resolution(unsigned int res)
{
  m_resolution = res;

  for (int i = 0; i < 6; i++)
  {
    if (m_voxel_maps[i])
      destroy_tex_3d(m_voxel_maps[i]);
    m_voxel_maps[i] = create_tex_3d(res, res, res, 7);
  }
}

void Renderer::set_grid_size(float size)
{
  m_cube_size = size;
}

void Renderer::set_rendering_phases(bool direct, bool diffuse, bool specular, bool shadow)
{
  m_enable_shadows = shadow;
  m_enable_direct = direct;
  m_enable_indirect_diffuse = diffuse;
  m_enable_indirect_specular = specular;
}

void Renderer::set_voxel_view_dir(int dir, float lod)
{
  m_view_voxel_dir = dir;
  m_view_voxel_lod = lod;
}

void Renderer::queue_model(model_id_t model_id)
{
  if (model_id >= m_models.size())
    return;

  model_t model = m_models[model_id];

  // Push model 
  m_draw_queue.push_back(model);
}

void Renderer::queue_point_light(point_light_t& point_light)
{
  m_point_lights.push_back(point_light);
}

void Renderer::set_model_transform(model_id_t model_id, glm::mat4 model_matrix)
{
  m_models[model_id].model_matrix = model_matrix;
}

material_t& Renderer::get_material(material_id_t material_id)
{
  return m_materials[material_id];
}

void Renderer::bind_material(material_t& material, int location)
{
  glBindBufferBase(GL_UNIFORM_BUFFER, location, material.ubo);
}

void Renderer::draw_models(const shader_t& shader)
{
  for (auto& model : m_draw_queue)
  {
    glUniformMatrix4fv(shader.model_location, 1, false, glm::value_ptr(model.model_matrix));

    glBindVertexArray(model.vao);

    for (size_t i = 0; i < model.draw_obj_range.size; i++)
    {
      draw_obj_t& draw_obj = m_draw_objs[model.draw_obj_range.start + i];
      // Bind ubos for materials
      bind_material(m_materials[draw_obj.material_id], shader.material_location);

      glDrawElements(draw_obj.draw_type, draw_obj.range.size, GL_UNSIGNED_INT, (const void*) (sizeof(unsigned) * draw_obj.range.start));
    }
  }
}

void Renderer::upload_lights(const shader_t& shader)
{
  for (size_t i = 0; i < m_point_lights.size(); i++)
  {
    int position_location = glGetUniformLocation(shader.program, ("point_lights[" + std::to_string(i) + "].position").c_str());
    int color_location = glGetUniformLocation(shader.program, ("point_lights[" + std::to_string(i) + "].color").c_str());
    int intensity_location = glGetUniformLocation(shader.program, ("point_lights[" + std::to_string(i) + "].intensity").c_str());

    glUniform3fv(position_location, 1, glm::value_ptr(m_point_lights[i].position));
    glUniform3fv(color_location, 1, glm::value_ptr(m_point_lights[i].color));
    glUniform1f(intensity_location, m_point_lights[i].intensity);
  }

  glUniform1i(glGetUniformLocation(shader.program, "point_light_count"), m_point_lights.size());
}

void Renderer::upload_camera(const shader_t& shader)
{
  glBindBufferBase(GL_UNIFORM_BUFFER, shader.camera_location, m_camera_ubo);

  glm::vec3 camera_position = glm::column(m_camera.view, 3);
  glUniform3fv(glGetUniformLocation(shader.program, "camera_position"), 1, glm::value_ptr(camera_position));
}

void Renderer::filter()
{
  glUseProgram(m_mipmap_shader);

  // Bind 3d texture
  for (int i = 0; i < 6; i++)
    activate_tex_3d(m_mipmap_shader, m_voxel_maps[i], i);
  glActiveTexture(GL_TEXTURE0);

  size_t current_dim = m_resolution;
  int mip = 0;

  int resolution_location = glGetUniformLocation(m_mipmap_shader, "resolution");
  int mip_location = glGetUniformLocation(m_mipmap_shader, "mip");

  while (current_dim >= 1)
  {
    glUniform1i(resolution_location, current_dim);
    glUniform1i(mip_location, mip);

    for (int i = 0; i < 6; i++)
      glBindImageTexture(i, m_voxel_maps[i], mip + 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);

    unsigned work_groups = static_cast<unsigned>(glm::ceil(current_dim / 8.0f));
    glDispatchCompute(work_groups, work_groups, work_groups);

    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    mip++;
    current_dim /= 2;
  }
}

void Renderer::voxelize()
{
  static GLfloat black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
  // Clear texture
  for (int i = 0; i < 6; i++)
    clear_tex_3d(m_voxel_maps[i], black);

  // Render scene
  shader_t& shader = m_shaders[m_voxelize_shader];
  glUseProgram(shader.program);
  glUniform1f(shader.cube_size_location, m_cube_size); 

  // Bind 3d texture
  int offset = 2;
  for (int i = 0; i < 6; i++)
    glBindImageTexture(i + offset, m_voxel_maps[i], 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);

  upload_camera(shader);

  // Upload lights
  upload_lights(shader);

  // Set render settings
  int viewport_res = m_resolution * 2;
  glViewport(0, 0, viewport_res, viewport_res);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  draw_models(shader);

  // Mipmap
  filter();

  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void Renderer::visualize()
{
  // Set settings
  glViewport(0, 0, m_viewport_width, m_viewport_height);

  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader_t& shader = m_shaders[m_draw_shader];
  glUseProgram(shader.program);
  glUniform1f(shader.cube_size_location, m_cube_size); 
  glUniform1i(shader.cube_res_location, m_resolution);

  glUniform1i(glGetUniformLocation(shader.program, "enable_diffuse"), m_enable_indirect_diffuse);
  glUniform1i(glGetUniformLocation(shader.program, "enable_specular"), m_enable_indirect_specular);
  glUniform1i(glGetUniformLocation(shader.program, "enable_shadow"), m_enable_shadows);
  glUniform1i(glGetUniformLocation(shader.program, "enable_direct"), m_enable_direct);

  glUniform1i(glGetUniformLocation(shader.program, "view_voxel_dir"), m_view_voxel_dir);
  glUniform1f(glGetUniformLocation(shader.program, "view_voxel_lod"), m_view_voxel_lod);

  // Bind 3d texture
  int offset = 2;
  for (int i = 0; i < 6; i++)
    activate_tex_3d(shader.program, m_voxel_maps[i], i + offset);

  upload_camera(shader);
  upload_lights(shader);

  glEnable(GL_DEPTH_TEST);
  //glEnable(GL_CULL_FACE);
  //glCullFace(GL_BACK);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  draw_models(shader);
}

void Renderer::render()
{
  glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera_data_t), &m_camera);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glClearColor(0.15, 0.25, 0.25, 1.0);

  voxelize();
  visualize();

  m_draw_queue.clear();
  m_point_lights.clear();
}

model_id_t Renderer::load_model(const char* filename)
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;

  std::string filename_obj = filename;
  std::string basepath = filename_obj.substr(0, filename_obj.find_last_of("\\/") + 1);

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename, basepath.c_str(), true))
  {
    fprintf(stderr, "Error loading file %s\n", filename);
    return INVALID_ID;
  }

  // Store materials
  size_t material_base_index = m_materials.size(); 
  for (const auto& material : materials)
  {
    material_t new_mat = {0};
    create_material(material, &new_mat);
    m_materials.push_back(new_mat);
  }

  std::vector<vert_data_t> vertices;
  std::vector<unsigned> indices;
  std::unordered_map<vert_data_t, unsigned> unique_verts = {};

  GLuint vao;
  GLuint vbo;
  GLuint ebo;

  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);

  draw_obj_t new_draw_obj;
  new_draw_obj.draw_type = GL_TRIANGLES;
  new_draw_obj.material_id = 0;
  new_draw_obj.range.start = 0;
  new_draw_obj.range.size = 0;

  model_t new_model;
  new_model.vao = vao;
  new_model.vbo = vbo;
  new_model.ebo = ebo;
  new_model.draw_obj_range.start = m_draw_objs.size();
  new_model.draw_obj_range.size = 0; 

  glm::vec3 min(FLT_MAX);
  glm::vec3 max(0);

  for (const auto& shape : shapes)
  { 
    int indices_base = indices.size();

    for (const auto& index : shape.mesh.indices)
    {
      vert_data_t vertex = {};

      vertex.pos = 
      {
        attrib.vertices[3 * index.vertex_index + 0],
        attrib.vertices[3 * index.vertex_index + 1],
        attrib.vertices[3 * index.vertex_index + 2]
      };

      if (vertex.pos.x > max.x)
        max.x = vertex.pos.x;
      if (vertex.pos.x < min.x)
        min.x = vertex.pos.x;
      if (vertex.pos.y > max.y) 
        max.y = vertex.pos.y;
      if (vertex.pos.y < min.y)
        min.y = vertex.pos.y;
      if (vertex.pos.z > max.z)
        max.z = vertex.pos.z;
      if (vertex.pos.z < min.y)
        min.z = vertex.pos.z; 

      if (index.normal_index >= 0)
      {
        vertex.norm = 
        {
          attrib.normals[3 * index.normal_index + 0],
          attrib.normals[3 * index.normal_index + 1],
          attrib.normals[3 * index.normal_index + 2]
        };
      }

      if (index.texcoord_index >= 0)
      {
        vertex.tex = 
        {
          attrib.texcoords[2 * index.texcoord_index + 0],
          attrib.texcoords[2 * index.texcoord_index + 1]
        };
      }

      if (unique_verts.count(vertex) == 0)
      {
        unique_verts[vertex] = vertices.size();
        vertices.push_back(vertex);
      }

      vertices.push_back(vertex);
      indices.push_back(unique_verts[vertex]);
    }

    new_model.dimensions = max - min;

    int current_material_id = shape.mesh.material_ids.at(0);
    size_t start = indices_base;
    size_t size = 0;

    for (size_t i = 0; i < shape.mesh.material_ids.size(); i++)
    {
      int material_id = shape.mesh.material_ids.at(i);

      if (material_id == -1)
        material_id = DEFAULT_MATERIAL_INDEX;

      if (current_material_id != material_id)
      {
        // push new draw object
        new_model.draw_obj_range.size++;

        new_draw_obj.range.start = start;
        new_draw_obj.range.size = size;
        new_draw_obj.material_id = material_base_index + current_material_id;

        m_draw_objs.push_back(new_draw_obj);

        // Initialize start + size for new object
        start += size;
        size = 0;
        current_material_id = material_id;
      }
      size += shape.mesh.num_face_vertices.at(i); 
    }

    // Add the last object

    new_model.draw_obj_range.size++;

    new_draw_obj.range.start = start;
    new_draw_obj.range.size = size;
    new_draw_obj.material_id = material_base_index + current_material_id;
    m_draw_objs.push_back(new_draw_obj);
  }

  // Upload everything to the device

  glBindVertexArray(new_model.vao);
  glBindBuffer(GL_ARRAY_BUFFER, new_model.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vert_data_t) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, new_model.ebo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * indices.size(), &indices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0,
      3,
      GL_FLOAT,
      false,
      sizeof(vert_data_t),
      NULL);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1,
      3,
      GL_FLOAT,
      false,
      sizeof(vert_data_t),
      (const void*) offsetof(vert_data_t, norm));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2,
      2,
      GL_FLOAT,
      false,
      sizeof(vert_data_t),
      (const void*) offsetof(vert_data_t, tex));

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

  // Finally, add the model to the vector of models
  m_models.push_back(new_model);

  return m_models.size() - 1;
}

shader_id_t Renderer::load_shader(const char* vertex_shader_name,
    const char* fragment_shader_name,
    const char* geometry_shader_name)
{
  GLuint vert_shader = load_shader_source(vertex_shader_name, GL_VERTEX_SHADER);
  GLuint frag_shader = load_shader_source(fragment_shader_name, GL_FRAGMENT_SHADER);
  GLuint geom_shader = load_shader_source(geometry_shader_name, GL_GEOMETRY_SHADER);

  GLuint shaders[3] = { vert_shader, frag_shader, geom_shader };

  int num_shaders = 3;

  if (!geometry_shader_name)
    num_shaders = 2;

  shader_t new_shader;
  new_shader.program = link_shader_program(shaders, num_shaders);
  new_shader.material_location = glGetUniformBlockIndex(new_shader.program, "material");
  new_shader.camera_location = glGetUniformBlockIndex(new_shader.program, "camera");
  new_shader.texture_3D_location = glGetUniformLocation(new_shader.program, "tex3D");

  new_shader.cube_size_location = glGetUniformLocation(new_shader.program, "cube_size");
  new_shader.cube_res_location = glGetUniformLocation(new_shader.program, "cube_res");
  new_shader.model_location = glGetUniformLocation(new_shader.program, "model");

  m_shaders.push_back(new_shader);  

  detach_shaders(new_shader.program, shaders, 2);

  destroy_shader(vert_shader);
  destroy_shader(frag_shader);

  if (geometry_shader_name)
    destroy_shader(geom_shader); 

  return m_shaders.size() - 1;
}

void Renderer::upload_material_data(material_data_t& material_data, material_id_t material_id)
{
  material_t& material = m_materials[material_id];
 
  glGenBuffers(1, &material.ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, material.ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(material_data_t), &material_data, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

material_id_t Renderer::add_material(material_data_t& material_data)
{
  material_t result;
  result.num_textures = 0;

  glGenBuffers(1, &result.ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, result.ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(material_data_t), &material_data, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  m_materials.push_back(result);
  return m_materials.size() - 1;
}

void Renderer::set_model_material(material_id_t material_id, model_id_t model_id)
{
  model_t& model = m_models[model_id];
  material_t& material = m_materials[material_id];

  for (int i = 0; i < model.draw_obj_range.size; i++)
  {
    draw_obj_t& draw_obj = m_draw_objs[i + model.draw_obj_range.start];
    draw_obj.material_id = material_id;
  }
}
 
