// TODO: Camera class

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl_utils/gl_helpers.h>

#include <stdio.h>
#include <stdlib.h>
#include "renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
  
  window = glfwCreateWindow(WIDTH, HEIGHT, "", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  
  glfwMakeContextCurrent(window);
  
  gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

  glfwSetKeyCallback(window, key_callback);

  renderer renderer;
  model_id_t model = renderer.load_model("assets/cornell-box.obj");
  shader_id_t shader = renderer.load_shader("shader/passthrough.vert", "shader/passthrough.frag");

  glm::vec3 camera_pos(-0.118010, 8.581252, 28.632803);
  glm::vec2 camera_rot(-90, 0);
 
  glm::mat4 perspective = glm::perspective(45.0f, (float) WIDTH / (float) HEIGHT, 1.0f, 100.0f);
  glm::mat4 init_view = glm::lookAt(camera_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

  renderer.set_camera_transform(init_view, perspective);

  static float init_move_speed = 0.01;
  static float init_camera_rot_amount = 0.01;

  double mouse_x, mouse_y;
  glfwGetCursorPos(window, &mouse_x, &mouse_y);

  glm::vec3 camera_up(0, 1, 0);

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

    camera_rot.x += camera_rot_amount * (new_mouse_x - mouse_x);
    camera_rot.y -= camera_rot_amount * (new_mouse_y - mouse_y);

    if (camera_rot.y > 89.0f)
      camera_rot.y = 89.0f;
    if (camera_rot.y < -89.0f)
      camera_rot.y = -89.0f;

    mouse_x = new_mouse_x;
    mouse_y = new_mouse_y;

    glm::vec3 camera_front;
    camera_front.x = cos(glm::radians(camera_rot.y)) * cos(glm::radians(camera_rot.x));
    camera_front.y = sin(glm::radians(camera_rot.y));
    camera_front.z = cos(glm::radians(camera_rot.y)) * sin(glm::radians(camera_rot.x));

    camera_front = glm::normalize(camera_front);

    glm::vec3 up = glm::vec3(0, 1, 0);

    int state = glfwGetKey(window, GLFW_KEY_D);
    if (state == GLFW_PRESS)
      camera_pos += move_speed * glm::normalize(glm::cross(camera_front, camera_up));

    state = glfwGetKey(window, GLFW_KEY_A);
    if (state == GLFW_PRESS)
      camera_pos -= move_speed * glm::normalize(glm::cross(camera_front, camera_up));

    state = glfwGetKey(window, GLFW_KEY_S);
    if (state == GLFW_PRESS)
      camera_pos -= move_speed * camera_front;

    state = glfwGetKey(window, GLFW_KEY_W);
    if (state == GLFW_PRESS)
      camera_pos += move_speed * camera_front;

    state = glfwGetKey(window, GLFW_KEY_SPACE);
    if (state == GLFW_PRESS)
      camera_pos.y += move_speed;

    state = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT);
    if (state == GLFW_PRESS)
      camera_pos.y -= move_speed;

    glm::mat4 view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up); 

    renderer.set_camera_transform(view, perspective);

    renderer.queue_model(model, shader, NULL, 0);
    renderer.render();
    glfwSwapBuffers(window);
  }

  return 0;
}
