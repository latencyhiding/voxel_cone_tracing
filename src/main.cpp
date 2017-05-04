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

static void error_callback(int error, const char *description)
{
  fprintf(stderr, "%s", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);
}

#define WIDTH 800
#define HEIGHT 600

#include <glm/gtx/string_cast.hpp>

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

  window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);

  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  glfwSetKeyCallback(window, key_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  Renderer renderer(WIDTH, HEIGHT);

  // Create scene
  model_id_t model = renderer.load_model("assets/CornellBox-Glossy.obj");

  model_id_t dragon = renderer.load_model("assets/dragon.obj");
  material_data_t dragon_material;
  dragon_material.ambient = glm::vec4(0.1, 0.3, 0.3, 1.0);
  dragon_material.diffuse = glm::vec4(0.1, 0.3, 0.3, 1.0);
  dragon_material.specular = glm::vec4(1.0, 1.0, 1.0, 1.0);
  dragon_material.transmittance = glm::vec4(1.0, 1.0, 1.0, 1.0);
  dragon_material.emission = glm::vec3(0.0, 0.0, 0.0); 
  dragon_material.shininess = 0;
  dragon_material.ior = 1.33;
  dragon_material.dissolve = 1;
  dragon_material.illum = 5;

  material_id_t dragon_material_id = renderer.add_material(dragon_material);
  renderer.set_model_material(dragon_material_id, dragon);
  glm::vec3 dragon_position = glm::vec3(0.75, 0.75, 0.75);

  renderer.set_grid_size(3);

  Camera camera(glm::vec3(0, .9, 3), 0, -90);
  camera.set_perspective(45.0f, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);
  renderer.set_camera_transform(camera.get_lookat(), camera.get_projection());

  point_light_t light;
  light.position = glm::vec3(0, 1.45, 0);
  light.color = glm::vec3(1.0f); //0.75, 0.75, 0.75);

  static float init_move_speed = 0.005;
  static float init_camera_rot_amount = 0.01;

  double mouse_x, mouse_y;
  glfwGetCursorPos(window, &mouse_x, &mouse_y);

  double last_time = glfwGetTime();

  while (!glfwWindowShouldClose(window))
  {
    double time = glfwGetTime();
    double dt = (time - last_time) * 1000.0f;
    last_time = time;

    float move_speed = dt * init_move_speed;
    float camera_rot_amount = dt * init_camera_rot_amount;

    // Input
    glfwPollEvents();
    double new_mouse_x, new_mouse_y;
    glfwGetCursorPos(window, &new_mouse_x, &new_mouse_y);

    camera.add_rot(-camera_rot_amount * (new_mouse_y - mouse_y)
        , camera_rot_amount * (new_mouse_x - mouse_x));

    mouse_x = new_mouse_x;
    mouse_y = new_mouse_y;

    float dx = 0, dy = 0, dz = 0;

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

    state = glfwGetKey(window, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS)
      dy = move_speed;

    state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (state == GLFW_PRESS)
      dy = -move_speed;

    camera.move(dx, dy, dz);
    //light.position += glm::vec3(0.2 * dx, 0.2 * dy, 0.2 * -dz);
    //renderer.set_camera_transform(camera.get_lookat(), camera.get_projection());

    dragon_position += glm::vec3(0.2 * dx, 0.2 * dy, 0.2 * -dz);
    glm::mat4 dragon_matrix;
    dragon_matrix = glm::translate(dragon_matrix, dragon_position);
    glm::mat4 dragon_rotate;
    dragon_matrix *= glm::rotate(dragon_rotate, (float) mouse_x / WIDTH, glm::vec3(0, 1, 0));
    renderer.set_model_transform(dragon, dragon_matrix);
    
    glm::mat4 model_matrix;
    renderer.set_model_transform(model, model_matrix);
    renderer.queue_model(model);
    renderer.queue_model(dragon);
    renderer.queue_point_light(light);

    renderer.render();
    glfwSwapBuffers(window);
  }

  return 0;
}
