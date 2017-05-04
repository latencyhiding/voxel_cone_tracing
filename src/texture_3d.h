#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

GLuint create_tex_3d(int width, int height, int depth, int levels);
void destroy_tex_3d(GLuint tex);
void activate_tex_3d(GLuint program, GLuint tex, GLuint unit);
void clear_tex_3d(GLuint tex, GLfloat clear_color[4]);
void mip_tex_3d(GLuint tex);
