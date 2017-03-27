#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <gl_utils/gl_helpers.h>

#include <stdio.h>
#include <stdlib.h>

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

int main()
{
  GLFWwindow* window;
  if (!glfwInit())
    exit(EXIT_FAILURE);
  
  glfwSetErrorCallback(error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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
  
  while (!glfwWindowShouldClose(window))
  {
    // Input
    glfwPollEvents();
  }
  
  return 0;
}
