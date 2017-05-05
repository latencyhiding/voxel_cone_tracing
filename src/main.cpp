// TODO: Camera class

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl_utils/gl_helpers.h>

#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui_impl_glfw_gl3.h>

static void error_callback(int error, const char *description)
{
  fprintf(stderr, "%s", description);
}

#define WIDTH 800 
#define HEIGHT 600 

#include <glm/gtx/string_cast.hpp>

#define CONTROL_WIDTH 400

namespace ImGui
{
  static auto vector_getter = [](void* vec, int idx, const char** out_text)
  {
    auto& vector = *static_cast<std::vector<std::string>*>(vec);
    if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
    *out_text = vector.at(idx).c_str();
    return true;
  };

  bool ListBox(const char* label, int* currIndex, std::vector<std::string>& values, int height_in_items)
  {
    if (values.empty()) { return false; }
    return ListBox(label, currIndex, vector_getter,
        static_cast<void*>(&values), values.size(), height_in_items);
  }
}

int main()
{
  GLFWwindow* window;
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

  window = glfwCreateWindow(WIDTH + CONTROL_WIDTH, HEIGHT, "", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  ImGui_ImplGlfwGL3_Init(window, true);

  Renderer renderer(WIDTH, HEIGHT);

  // Create scene
  model_id_t box = renderer.load_model("assets/CornellBox-Glossy.obj");

  model_id_t dynamic_object = renderer.load_model("assets/suzanne.obj");
  material_data_t dynamic_object_material;
  dynamic_object_material.ambient = glm::vec4(1.0);
  dynamic_object_material.diffuse = glm::vec4(0.0);
  dynamic_object_material.specular = glm::vec4(1.0);
  dynamic_object_material.transmittance = glm::vec4(1.0);
  dynamic_object_material.emission = glm::vec3(0.0, 0.0, .25); 
  dynamic_object_material.shininess = 1000;
  dynamic_object_material.ior = 5;
  dynamic_object_material.dissolve = 0.1;
  dynamic_object_material.illum = 4;

  material_id_t dynamic_object_material_id = renderer.add_material(dynamic_object_material);
  renderer.set_model_material(dynamic_object_material_id, dynamic_object);
  glm::vec3 dynamic_object_position = glm::vec3(0, 1.1, -0.5);
  float dynamic_object_scale = 0.3;
  float dynamic_object_rot = 0;

  renderer.set_grid_size(3);

  Camera camera(glm::vec3(0, .9, 3), 0, -90);
  camera.set_perspective(45.0f, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
  renderer.set_camera_transform(camera.get_lookat(), camera.get_projection());

  std::vector<std::string> light_names;
  std::vector<point_light_t> lights;
  int current_light_index = 0;

  point_light_t ceiling_light;
  ceiling_light.position = glm::vec3(0, 1.4, 0);
  ceiling_light.color = glm::vec3(1.0f);
  ceiling_light.intensity = 1.0;

  light_names.push_back("Ceiling light");
  lights.push_back(ceiling_light);

  static float init_move_speed = 0.005;
  static float init_rot_speed = 0.005;
  static float init_camera_rot_amount = 0.01;

  double mouse_x, mouse_y;
  glfwGetCursorPos(window, &mouse_x, &mouse_y);

  double last_time = glfwGetTime();

  bool control_object = false;
  bool control_light = false;
  bool control_camera = false;

  bool enable_diffuse = true;
  bool enable_specular = true;
  bool enable_shadows = true;
  bool enable_direct = true;

  while (!glfwWindowShouldClose(window))
  {
    double time = glfwGetTime();
    double dt = (time - last_time) * 1000.0f;
    last_time = time;

    float move_speed = dt * init_move_speed;
    float turn_speed = dt * init_rot_speed;
    float camera_rot_amount = dt * init_camera_rot_amount;

    // Input
    glfwPollEvents();
    ImGui_ImplGlfwGL3_NewFrame();

    // Menu

    point_light_t& current_light = lights[current_light_index];

    {
      ImGui::SetNextWindowSizeConstraints(ImVec2(CONTROL_WIDTH, HEIGHT), ImVec2(CONTROL_WIDTH, HEIGHT));
      ImGui::SetNextWindowPos(ImVec2(WIDTH, 0));
      ImGui::Begin("CONTROL");

      ImGui::Text("Dynamic object parameters");
      ImGui::SliderFloat3("Diffuse", glm::value_ptr(dynamic_object_material.diffuse), 0.0, 1.0);
      ImGui::SliderFloat3("Specular", glm::value_ptr(dynamic_object_material.specular), 0.0, 1.0);
      ImGui::SliderFloat3("Transmittance", glm::value_ptr(dynamic_object_material.transmittance), 0.0, 1.0);
      ImGui::SliderFloat3("Emission", glm::value_ptr(dynamic_object_material.emission), 0.0, 1.0);
      ImGui::SliderFloat("Shininess", &dynamic_object_material.shininess, 1, 1000);
      ImGui::SliderFloat("Index of Refraction", &dynamic_object_material.ior, 1, 5);
      ImGui::SliderFloat("Opacity", &dynamic_object_material.dissolve, 0, 1);

      bool transparency = dynamic_object_material.illum == 4;
      ImGui::Checkbox("Refraction", &transparency);
      if (transparency)
        dynamic_object_material.illum = 4;
      else
        dynamic_object_material.illum = 5;

      ImGui::Separator();

      ImGui::Text("Light parameters");
      ImGui::ListBox("Lights", &current_light_index, light_names, 3);
      ImGui::Text("Current light: %s", light_names[current_light_index].c_str());
      ImGui::SliderFloat3("Light color", glm::value_ptr(current_light.color), 0.0, 1.0);
      ImGui::SliderFloat("Light intensity", &current_light.intensity, 0.0, 1.0);

      ImGui::Text("Create light");
      static char light_name[256];
      bool light_name_filled = ImGui::InputText("Light name", light_name, 256);
      if (ImGui::Button("New light"))
      {
        light_names.push_back(light_name);

        point_light_t new_light;
        new_light.color = glm::vec3(1.0, 1.0, 1.0);
        new_light.position = glm::vec3(0, 1.45, 0);
        new_light.intensity = 0;

        lights.push_back(new_light);
      }

      ImGui::SameLine();

      if (ImGui::Button("Delete light"))
      {
        lights.erase(lights.begin() + current_light_index);
        light_names.erase(light_names.begin() + current_light_index);
      }

      ImGui::Separator();

      static const char* control_options[] =
      {
        "object",
        "light",
        "camera",
        "none"
      };

      static int current_control = 3;
      
      ImGui::Text("Transform control: %s. Press ESC to stop control", control_options[current_control]);

      if (ImGui::Button("Dynamic object (WASD move, QE rotate)"))
      {
        control_object = true;
        control_light = false;
        control_camera = false;
        current_control = 0;
      }

      if (ImGui::Button("Current light (WASD move)"))
      {
        control_object = false;
        control_light = true;
        control_camera = false;
        current_control = 1;
      }

      if (ImGui::Button("Camera (WASD move, mouse look)"))
      {
        control_object = false;
        control_light = false;
        control_camera = true;
        current_control = 2;
      }

      if (ImGui::Button("None") || ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape)))
      {
        control_object = false;
        control_light = false;
        control_camera = false;
        current_control = 3;
      }

      ImGui::Separator();

      ImGui::Text("Rendering stages");
      ImGui::Checkbox("Enable indirect diffuse", &enable_diffuse);
      ImGui::Checkbox("Enable indirect specular", &enable_specular);
      ImGui::Checkbox("Enable soft shadows", &enable_shadows);
      ImGui::Checkbox("Enable direct", &enable_direct);

      ImGui::Separator();

      ImGui::Text("View filtered voxel map");

      static bool view_voxels = false;
      ImGui::Checkbox("Enable", &view_voxels);

      static float lod = 0;
      static int dir = 0;

      if (view_voxels)
      {
        ImGui::SliderFloat("LOD", &lod, 0, 7);
        ImGui::Text("View direction: "); ImGui::SameLine();
        if (ImGui::Button("x"))
          dir = 1;
        ImGui::SameLine(); 
        if (ImGui::Button("-x"))
          dir = 0;
        ImGui::SameLine(); 
        if (ImGui::Button("y"))
          dir = 3;
        ImGui::SameLine(); 
        if (ImGui::Button("-y"))
          dir = 2;
        ImGui::SameLine(); 
        if (ImGui::Button("z"))
          dir = 5;
        ImGui::SameLine(); 
        if (ImGui::Button("-z"))
          dir = 4;
        renderer.set_voxel_view_dir(dir, lod);
      }
      else
        renderer.set_voxel_view_dir(7, lod);


      ImGui::End();
    }

    renderer.upload_material_data(dynamic_object_material, dynamic_object_material_id);
    renderer.set_rendering_phases(enable_direct, enable_diffuse, enable_specular, enable_shadows);

    double new_mouse_x, new_mouse_y;
    glfwGetCursorPos(window, &new_mouse_x, &new_mouse_y);

    float dx = 0, dy = 0, dz = 0;
    float dtheta = 0;

    int state = glfwGetKey(window, GLFW_KEY_D);
    if (state == GLFW_PRESS)
      dx = move_speed;

    state = glfwGetKey(window, GLFW_KEY_A);
    if (state == GLFW_PRESS)
      dx = -move_speed;

    state = glfwGetKey(window, GLFW_KEY_S);
    if (state == GLFW_PRESS)
      dz = -move_speed;

    state = glfwGetKey(window, GLFW_KEY_W);
    if (state == GLFW_PRESS)
      dz = move_speed;

    state = glfwGetKey(window, GLFW_KEY_Q);
    if (state == GLFW_PRESS)
      dtheta = turn_speed;

    state = glfwGetKey(window, GLFW_KEY_E);
    if (state == GLFW_PRESS)
      dtheta = -turn_speed;

    state = glfwGetKey(window, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS)
      dy = move_speed;

    state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (state == GLFW_PRESS)
      dy = -move_speed;

    if (control_camera)
    {
      camera.move(dx, dy, dz);
      camera.add_rot(-camera_rot_amount * (new_mouse_y - mouse_y), camera_rot_amount * (new_mouse_x - mouse_x));
      renderer.set_camera_transform(camera.get_lookat(), camera.get_projection());
    }
    else if (control_object)
    {
      dynamic_object_position += glm::vec3(0.2 * dx, 0.2 * dy, 0.2 * -dz);
      dynamic_object_rot += dtheta;
    }
    else if (control_light)
    {
      point_light_t& current_light = lights[current_light_index];
      current_light.position += glm::vec3(0.2 * dx, 0.2 * dy, 0.2 * -dz);
    }

    mouse_x = new_mouse_x;
    mouse_y = new_mouse_y;

    // Renderer

    glm::mat4 dynamic_object_matrix;
    dynamic_object_matrix = glm::translate(dynamic_object_matrix, dynamic_object_position);
    dynamic_object_matrix = glm::rotate(dynamic_object_matrix, dynamic_object_rot, glm::vec3(0, 1, 0));
    dynamic_object_matrix = glm::scale(dynamic_object_matrix, glm::vec3(dynamic_object_scale, dynamic_object_scale, dynamic_object_scale));
    glm::mat4 dynamic_object_rotate;
    renderer.set_model_transform(dynamic_object, dynamic_object_matrix);

    glm::mat4 box_matrix;
    renderer.set_model_transform(box, box_matrix);
    renderer.queue_model(box);
    renderer.queue_model(dynamic_object);

    for (auto& light : lights)
      renderer.queue_point_light(light);

    renderer.render();

    ImGui::Render();
    glfwSwapBuffers(window);
  }

  ImGui_ImplGlfwGL3_Shutdown();

  return 0;
}
