#include <gl_utils/gl_helpers.h>
#include <string.h>

// Vertex Specs
void create_vert_attrib(vert_attrib_t* attrib,
                        const char* name,
                        int location,
                        int size,
                        GLenum type,
                        bool normalize,
                        size_t stride,
                        size_t offset)
{
  if (strlen(name) > 256)
    return;
  strcpy(attrib->name, name);
  attrib->location = location;
  attrib->size = size;
  attrib->type = type;
  attrib->normalize = normalize;
  attrib->stride = stride;
  attrib->offset = offset;
}

void set_vertex_spec(vert_attrib_t* attribs, size_t num_attribs)
{
  for (int i = 0; i < num_attribs; i++)
  {
    vert_attrib_t* attrib = attribs + i;
    
    glEnableVertexAttribArray(attrib->location);
    glVertexAttribPointer(attrib->location, attrib->size, attrib->type,
                          attrib->normalize, attrib->stride, (void*) attrib->offset);
 
  }
}

GLuint create_buffer(const void* data, size_t data_size, GLenum usage, GLenum access_type)
{
  GLuint buf;
  glGenBuffers(1, &buf);
  glBindBuffer(usage, buf);
  glBufferData(usage, data_size, data, access_type);
  glBindBuffer(usage, 0);

  return buf;
}

void delete_buffer(GLuint buf)
{
  glDeleteBuffers(1, &buf);
}

// Shaders

#define ERROR_BUFF_SIZE 512

GLuint compile_shader(const char* source, size_t size, GLenum shader_type)
{
  GLuint result = glCreateShader(shader_type);
  glShaderSource(result, 1, (const GLchar**) &source, (int*) &size);
  glCompileShader(result);

  int is_compiled;
  glGetShaderiv(result, GL_COMPILE_STATUS, (int*) &is_compiled);

  if (!(is_compiled == GL_TRUE))
  {
    fprintf(stderr, "Compile issue:");
    char error_buff[ERROR_BUFF_SIZE];
    int max_length = 0;
    glGetShaderiv(result, GL_INFO_LOG_LENGTH, &max_length);
    glGetShaderInfoLog(result, max_length, &max_length, error_buff);

    error_buff[max_length] = 0;
    fprintf(stderr, "%s\n", error_buff);
    
    glDeleteShader(result);
    result = 0;
  }
  return result;
}

GLuint load_shader_source(const char* pathname, GLenum shader_type)
{
  if (!pathname)
    return 0;

  GLuint result;
  FILE* file = NULL;
  size_t file_size;
  char* shader_string_buffer;

  file = fopen(pathname, "r+");
  if (!file)
  {
    fprintf(stderr, "Unable to load file %s.\n", pathname);
    return 0;
  }
  fseek(file, 0L, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  shader_string_buffer = (char*) malloc(file_size + 1);
  if (fread(shader_string_buffer, 1, file_size, file) != file_size)
  {
    fprintf(stderr, "Read error when reading %s.\n", pathname);
    return 0;
  }
  shader_string_buffer[file_size] = 0;

  result = compile_shader(shader_string_buffer, file_size, shader_type);

  free(shader_string_buffer);
  fclose(file);
  return result;
}

void destroy_shader(GLuint shader)
{
  if (!shader)
    return;

  glDeleteShader(shader);
}

GLuint link_shader_program(GLuint* shaders, size_t num_shaders)
{
  if (!shaders)
    return 0;

  GLuint program = glCreateProgram();
  for (size_t i = 0; i < num_shaders; i++)
  {
    if (!shaders[i])
      continue;
    glAttachShader(program, shaders[i]);
  }

  glLinkProgram(program);

  int is_linked;
  glGetProgramiv(program, GL_LINK_STATUS, &is_linked);

  if (!is_linked)
  {
    fprintf(stderr, "Linking error\n");

    char error_buff[ERROR_BUFF_SIZE];
    int max_length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length);
    glGetProgramInfoLog(program, max_length, &max_length, error_buff);

    error_buff[max_length] = 0;
    fprintf(stderr, "%s\n", error_buff);

    glDeleteProgram(program);
    return 0;
  }

  return program;
}

void destroy_program(GLuint program)
{
  glDeleteProgram(program);
}

void detach_shaders(GLuint program, GLuint* shaders, size_t num_shaders)
{
  for (size_t i = 0; i < num_shaders; i++)
  {
    glDetachShader(program, shaders[i]);
  }
}

