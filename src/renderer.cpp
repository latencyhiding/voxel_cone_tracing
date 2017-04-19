// TODO: Split into pass class
// TODO: Finish shader implementation
// TODO: Write your own damn object loader
// TODO: Support textures
// TODO: Materials should not just bind 2D textures

#include <tinyobjloader/tiny_obj_loader.h>
#include <unordered_map>

#include "renderer.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <gl_utils/gl_helpers.h>

typedef float float3[3];
typedef float float2[2];

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

#pragma pack(push, 1)
typedef struct 
{
  float ambient[4];
  float diffuse[4];
  float specular[4];
  float transmittance[4];
  float emission[4];
  float shininess;
  float ior;       // index of refraction
  float dissolve;  // 1 == opaque; 0 == fully transparent

  // PBR extension
  float roughness;            // [0, 1] default 0
  float metallic;             // [0, 1] default 0
  float sheen;                // [0, 1] default 0
  float clearcoat_thickness;  // [0, 1] default 0
  float clearcoat_roughness;  // [0, 1] default 0
  float anisotropy;           // aniso. [0, 1] default 0
  float anisotropy_rotation;  // anisor. [0, 1] default 0
} material_data_t;
#pragma pack(pop)

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
  mat = {0};
  mat.ambient[0] = 255;
  mat.ambient[1] = 0;
  mat.ambient[2] = 255;
  mat.diffuse[0] = 255;
  mat.diffuse[1] = 0;
  mat.diffuse[2] = 255;

  mat.shininess = 0;
  mat.ior = 0.4;
  mat.dissolve = 1;
}

renderer::renderer()
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
}

renderer::~renderer()
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
}

model_id_t renderer::load_model(const char* filename)
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
  new_draw_obj.vao = vao;
  new_draw_obj.vbo = vbo;
  new_draw_obj.ebo = ebo;
  new_draw_obj.draw_type = GL_TRIANGLES;
  new_draw_obj.shader_id = INVALID_ID;
  new_draw_obj.material_index = 0;
  new_draw_obj.range.start = 0;
  new_draw_obj.range.size = 0;

  model_t new_model;
  new_model.vao = vao;
  new_model.vbo = vbo;
  new_model.ebo = ebo;
  new_model.draw_objs_start_index = m_draw_objs.size();
  new_model.num_draw_objs = 0;

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
        new_model.num_draw_objs++;

        new_draw_obj.range.start = start;
        new_draw_obj.range.size = size;
        new_draw_obj.material_index = material_base_index + current_material_id;

        m_draw_objs.push_back(new_draw_obj);

        // Initialize start + size for new object
        start += size;
        size = 0;
        current_material_id = material_id;
      }
      size += shape.mesh.num_face_vertices.at(i); 
    }

    // Add the last object

    new_model.num_draw_objs++;

    new_draw_obj.range.start = start;
    new_draw_obj.range.size = size;
    new_draw_obj.material_index = material_base_index + current_material_id;
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

shader_id_t renderer::load_shader(const char* vertex_shader_name,
    const char* fragment_shader_name,
    const char* geometry_shader_name)
{
  GLuint vert_shader = load_shader_source(vertex_shader_name, GL_VERTEX_SHADER);
  GLuint frag_shader = load_shader_source(fragment_shader_name, GL_FRAGMENT_SHADER);
  GLuint geom_shader = load_shader_source(geometry_shader_name, GL_GEOMETRY_SHADER);

  GLuint shaders[3] = { vert_shader, frag_shader, geom_shader };

  shader_t new_shader;
  new_shader.program = link_shader_program(shaders, 3);
  new_shader.material_location = glGetUniformBlockIndex(new_shader.program, "material");
  new_shader.camera_location = glGetUniformBlockIndex(new_shader.program, "camera");

  m_shaders.push_back(new_shader);  

  detach_shaders(new_shader.program, shaders, 3);

  destroy_shader(vert_shader);
  destroy_shader(frag_shader);
  destroy_shader(geom_shader); 
}

int renderer::shader_get_uniform_location(shader_id_t shader_id, const char* name)
{
  return glGetUniformLocation(m_shaders[shader_id].program, name);
}

void renderer::set_camera_transform(glm::mat4& lookat, glm::mat4& projection)
{
  m_camera.view = lookat;
  m_camera.projection = projection;
}

void renderer::queue_model(model_id_t model_id, shader_id_t shader_id, uniform_t* uniforms, size_t num_uniforms)
{
  if (model_id >= m_models.size())
    return;

  model_t& model = m_models[model_id];

  buffer_range_t uniform_range;
  uniform_range.start = m_uniforms.size();
  uniform_range.size = num_uniforms;

  // Push uniforms
  for (size_t i = 0; i < num_uniforms; i++)
    m_uniforms.push_back(uniforms[i]);

  // Push draw objects
  for (size_t i = 0; i < model.num_draw_objs; i++)
  {
    draw_obj_t& draw_obj = m_draw_objs[i + model.draw_objs_start_index];
    draw_obj.shader_id = shader_id;
    draw_obj.uniform_range = uniform_range;

    m_draw_queue.push_back(draw_obj);
  }
}

void renderer::upload_uniforms(buffer_range_t& uniforms_range)
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

void renderer::bind_material(material_t& material, int location)
{
  glBindBufferBase(GL_UNIFORM_BUFFER, location, material.ubo);

  for (size_t i = 0; i < material.num_textures; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, material.textures[i]);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void renderer::render()
{
  glBindBuffer(GL_UNIFORM_BUFFER, m_camera_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(camera_data_t), &m_camera);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glClearColor(0.15, 0.25, 0.25, 1.0);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  draw_obj_t cached_draw_obj = {0};
  cached_draw_obj.shader_id = INVALID_ID;

  for (auto& draw_obj : m_draw_queue)
  {
    // Bind vao
    if (draw_obj.vao != cached_draw_obj.vao)
    {
      cached_draw_obj.vao = draw_obj.vao;
      glBindVertexArray(draw_obj.vao);
    }

    // Bind shaders
    if (draw_obj.shader_id != cached_draw_obj.shader_id)
    {
      cached_draw_obj.shader_id = draw_obj.shader_id;

      shader_t& shader = m_shaders[draw_obj.shader_id];
      glUseProgram(shader.program);

      // Bind ubos for camera and materials
      glBindBufferBase(GL_UNIFORM_BUFFER, shader.camera_location, m_camera_ubo);
      bind_material(m_materials[draw_obj.material_index], shader.material_location);
    }

    // Bind uniforms, bind materials
    if (draw_obj.material_index != cached_draw_obj.material_index)
    {
      cached_draw_obj.material_index = draw_obj.material_index;

      shader_t& shader = m_shaders[draw_obj.shader_id];
      material_t& material = m_materials[draw_obj.material_index];

      bind_material(material, shader.material_location);
    }

    upload_uniforms(draw_obj.uniform_range);

    glDrawElements(draw_obj.draw_type, draw_obj.range.size, GL_UNSIGNED_INT, (const void*) (sizeof(unsigned) * draw_obj.range.start));
  }

  m_uniforms.clear();
  m_draw_queue.clear();
}
