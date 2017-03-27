#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

typedef struct
{
  char name[256];
  int location;
  
  size_t size;
  GLenum type;
  bool normalize;
  size_t stride;
  size_t offset;
} vert_attrib_t;

typedef struct
{
  vert_attrib_t* attribs;
  size_t num_attribs;
} vert_spec_t;

void create_vert_attrib(vert_attrib_t* attrib,
                        const char* name,
                        int location,
                        size_t size,
                        GLenum type,
                        bool normalize,
                        size_t stride,
                        size_t offset);

void create_vertex_spec(vert_spec_t* vert_spec, vert_attrib_t* attribs, size_t num_attribs);

void set_vertex_spec(vert_spec_t* vert_spec);

GLuint create_buffer(const void* data, size_t data_size, GLenum usage, GLenum access_type);

void delete_buffer(GLuint buf);

// Shaders

GLuint compile_shader(const char* source, size_t size, GLenum shader_type);

GLuint load_shader_source(const char* pathname, GLenum shader_type);

void destroy_shader(GLuint shader);

GLuint link_shader_program(GLuint* shaders, size_t num_shaders, vert_spec_t* vert_spec);

void destroy_program(GLuint program);

void detach_shaders(GLuint program, GLuint* shaders, size_t num_shaders);

