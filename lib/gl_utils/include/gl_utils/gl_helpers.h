#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  char name[256];
  int location;
  int size;
  GLenum type;
  bool normalize;
  size_t stride;
  size_t offset;
} vert_attrib_t;

void create_vert_attrib(vert_attrib_t* attrib,
                        const char* name,
                        int location,
                        int size,
                        GLenum type,
                        bool normalize,
                        size_t stride,
                        size_t offset);

void set_vertex_spec(vert_attrib_t* vert_spec, size_t num_attribs);

GLuint create_buffer(const void* data, size_t data_size, GLenum usage, GLenum access_type);

void delete_buffer(GLuint buf);

// Shaders

GLuint compile_shader(const char* source, size_t size, GLenum shader_type);

GLuint load_shader_source(const char* pathname, GLenum shader_type);

void destroy_shader(GLuint shader);

GLuint link_shader_program(GLuint* shaders, size_t num_shaders);

void destroy_program(GLuint program);

void detach_shaders(GLuint program, GLuint* shaders, size_t num_shaders);

#ifdef __cplusplus
}
#endif

