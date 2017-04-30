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
  model_id_t model = renderer.load_model("assets/CornellBox-Glossy.obj");
  glm::vec3 dims = renderer.get_model_dimensions(model);
  renderer.set_grid_size(2);

  Camera camera(glm::vec3(0, .75, 3), 0, -90);
  camera.set_perspective(45.0f, (float) WIDTH / (float) HEIGHT, 0.1f, 100.0f);

  point_light_t light;
  light.position = glm::vec3(15, 15, 0);
  light.color = glm::vec3(1.0, 1.0, 1.0);

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
    light.position = camera.m_pos;

    renderer.set_camera_transform(camera.get_lookat(), camera.get_projection());

    glm::mat4 model_matrix;
    renderer.set_model_transform(model, model_matrix);
    renderer.queue_model(model);
    renderer.queue_point_light(light);

    renderer.render();
    glfwSwapBuffers(window);
  }

  return 0;
}
