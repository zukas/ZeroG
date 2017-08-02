#include "renderer.h"

#include "../Utils/log.h"

#include <GL/glew.h>
#define Gl_HEADER_PRIORITY
#include <GL/gl.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

static GLFWwindow *window{nullptr};

#ifndef NDEBUG
static const char *debug_source(GLenum source) {
  switch (source) {
  case GL_DEBUG_SOURCE_API:
    return "API";
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
    return "Window System";
  case GL_DEBUG_SOURCE_SHADER_COMPILER:
    return "Shader Compiler";
  case GL_DEBUG_SOURCE_THIRD_PARTY:
    return "Third Party";
  case GL_DEBUG_SOURCE_APPLICATION:
    return "Application";
  case GL_DEBUG_SOURCE_OTHER:
  default:
    return "Other";
  }
}

static const char *debug_type(GLenum type) {
  switch (type) {
  case GL_DEBUG_TYPE_ERROR:
    return "Error";
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
    return "Deprecated Behaviour";
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
    return "Undefined Behaviour";
  case GL_DEBUG_TYPE_PORTABILITY:
    return "Portability";
  case GL_DEBUG_TYPE_PERFORMANCE:
    return "Performance";
  case GL_DEBUG_TYPE_MARKER:
    return "Marker";
  case GL_DEBUG_TYPE_PUSH_GROUP:
    return "Push Group";
  case GL_DEBUG_TYPE_POP_GROUP:
    return "Pop Group";
  case GL_DEBUG_TYPE_OTHER:
  default:
    return "Other";
  }
}

static const char *debug_severity(GLenum severity) {
  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    return "high";
  case GL_DEBUG_SEVERITY_MEDIUM:
    return "medium";
  case GL_DEBUG_SEVERITY_LOW:
    return "low";
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    return "notification";
  default:
    return "none";
  }
}

static void APIENTRY debug_output(GLenum source, GLenum type, GLuint id,
                                  GLenum severity, GLsizei length,
                                  const GLchar *message,
                                  const void *userParam) {
  // ignore non-significant error/warning codes
  if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
    return;

  constexpr int32_t max_debug_buffer{4096};
  char debug_buffer[max_debug_buffer];
  const int32_t debug_length =
      snprintf(debug_buffer, max_debug_buffer,
               "------------------------------------------------ DEBUG "
               "------------------------------------------------\n"
               "ID: %u, SOURCE: %s, TYPE: %s, SEVERITY: %s\n"
               "%s\n"
               "---------------------------------------------------------------"
               "----------------------------------------\n",
               id, debug_source(source), debug_type(type),
               debug_severity(severity), message);
  logger::write_err(debug_buffer, debug_length);
}
#endif

bool renderer::init() {

  bool res = glfwInit();

  if (res) {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    const auto monitor = glfwGetPrimaryMonitor();
    const auto mode = glfwGetVideoMode(monitor);
    window =
        glfwCreateWindow(mode->width, mode->height, "Test 1", monitor, nullptr);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
      glfwDestroyWindow(window);
      window = nullptr;
      res = false;
    }
  }

#ifndef NDEBUG
  if (res) {
    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(debug_output, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
                            nullptr, GL_TRUE);
    }
  }
#endif
  //  glfwSwapInterval(0);

  return res;
}

void renderer::deinit() {
  glfwDestroyWindow(window);
  window = nullptr;
  glfwTerminate();
}

void renderer::enable_cursor(bool enable) {
  glfwSetInputMode(window, GLFW_CURSOR, enable);
}
void renderer::enable_sticky_keys(bool enable) {
  glfwSetInputMode(window, GLFW_STICKY_KEYS, enable);
}

void renderer::enable_sticky_mouse(bool enable) {
  glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, enable);
}

uint32_t renderer::create_shader_program(const common::ShaderSource *shaders,
                                         int32_t size) {
  uint32_t program_id = glCreateProgram();

  int32_t status{GL_FALSE};
  constexpr int32_t max_log_size{2048};

  for (int32_t i = 0; i < size; ++i) {

    uint32_t shader_id = glCreateShader(shaders[i].type);
    const char *source = shaders[i].source;

    glShaderSource(shader_id, 1, &source, nullptr);
    glCompileShader(shader_id);
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

    if (status) {
      glAttachShader(program_id, shader_id);
      glDeleteShader(shader_id);
    } else {
      int log_size{0};
      glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_size);
      if (log_size > 0) {
        char error[max_log_size]{0};
        log_size = log_size < max_log_size ? log_size : max_log_size;
        glGetShaderInfoLog(shader_id, log_size, nullptr, error);
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR,
                             0, GL_DEBUG_SEVERITY_MEDIUM, log_size, error);
      }
      glDeleteShader(shader_id);
      glDeleteProgram(program_id);
      program_id = 0;
      break;
    }
  }

  if (program_id > 0) {
    glLinkProgram(program_id);
    glGetProgramiv(program_id, GL_LINK_STATUS, &status);
    if (!status) {
      int log_size{0};
      glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_size);
      if (log_size > 0) {
        char error[max_log_size]{0};
        log_size = log_size < max_log_size ? log_size : max_log_size;
        glGetProgramInfoLog(program_id, log_size, nullptr, error);
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_ERROR,
                             0, GL_DEBUG_SEVERITY_MEDIUM, log_size, error);
      }
      glDeleteProgram(program_id);
      program_id = 0;
    }
  }

  return program_id;
}

void renderer::destroy_shader_program(uint32_t shader_program_id) {
  glDeleteProgram(shader_program_id);
}

void renderer::create_vertex_array(uint32_t *vertex_array, int32_t size) {
  glGenVertexArrays(size, vertex_array);
}

void renderer::destry_vertex_array(const uint32_t *vertex_array, int32_t size) {
  glDeleteVertexArrays(size, vertex_array);
}

void renderer::create_buffers(uint32_t *buffers, int32_t size) {
  glGenBuffers(size, buffers);
}

void renderer::destry_buffers(const uint32_t *buffers, int32_t size) {
  glDeleteBuffers(size, buffers);
}

uint32_t renderer::resolve_uniform_block_index(uint32_t program_id,
                                               const char *block_name) {
  return glGetUniformBlockIndex(program_id, block_name);
}

void renderer::bind_uniform_block(uint32_t program_id, uint32_t buffer_id,
                                  uint32_t block_index, uint32_t bind_point,
                                  const void *data, uint32_t size) {
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
  glUniformBlockBinding(program_id, block_index, bind_point);

  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, bind_point, buffer_id);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

uint32_t
renderer::bind_uniform_block(uint32_t program_id, uint32_t buffer_id,
                             const common::UniformBlock *uniform_block) {
  uint32_t block_index =
      glGetUniformBlockIndex(program_id, uniform_block->block_name);
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
  glUniformBlockBinding(program_id, block_index, uniform_block->bind_point);
  glBufferData(GL_UNIFORM_BUFFER, uniform_block->block_size, nullptr,
               GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, uniform_block->bind_point, buffer_id);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  return block_index;
}

void renderer::update_uniform_buffer(uint32_t buffer_id, const void *data,
                                     int32_t size, int32_t offset) {
  glBindBuffer(GL_UNIFORM_BUFFER, buffer_id);
  glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void renderer::bind_vertex_array(uint32_t vertex_array, uint32_t vertex_buffer,
                                 const void *data, int32_t size,
                                 int32_t elem_size,
                                 const common::VertexAttr *attr,
                                 int32_t attr_count) {
  glBindVertexArray(vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

  glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  int32_t offset{0};
  for (int32_t i = 0; i < attr_count; i++) {
    const common::VertexAttr &current = attr[i];
    glEnableVertexAttribArray(i);
    glVertexAttribPointer(
        i,                                     // attribute
        current.count,                         // size
        current.type,                          // type
        current.normalized,                    // normalized?
        elem_size,                             // stride
        reinterpret_cast<const void *>(offset) // array buffer offset
        );
    offset += current.count * 4;
  }
  glBindVertexArray(0);
}

void renderer::update_vertex_array(uint32_t vertex_array,
                                   uint32_t vertex_buffer, const void *data,
                                   int32_t size, int32_t offset) {
  glBindVertexArray(vertex_array);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
  glBindVertexArray(0);
}

void renderer::bind_index_array(uint32_t vertex_array, uint32_t index_buffer,
                                const void *data, int32_t size) {
  glBindVertexArray(vertex_array);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
  glBindVertexArray(0);
}

void renderer::update_index_array(uint32_t vertex_array, uint32_t index_buffer,
                                  const void *data, int32_t size,
                                  int32_t offset) {

  glBindVertexArray(vertex_array);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
  glBindVertexArray(0);
}

void renderer::use_program(uint32_t program_id) { glUseProgram(program_id); }

void renderer::draw_vertex_array(uint32_t vertex_array, int32_t vertex_count) {
  glBindVertexArray(vertex_array);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count);
  glBindVertexArray(0);
}

void renderer::draw_vertex_array_indexed(uint32_t vertex_array,
                                         int32_t elem_count) {
  glBindVertexArray(vertex_array);
  glDrawElements(GL_TRIANGLES, elem_count, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
}

void renderer::set_clear_colour(glm::vec4 colour) {
  glm::vec4 normal = glm::normalize(colour);
  glClearColor(normal.r, normal.g, normal.b, normal.a);
}

void renderer::set_clear_depth(float depth) { glClearDepth(depth); }

void renderer::set_depth_lt() { glDepthFunc(GL_LESS); }

void renderer::set_depth_gt() { glDepthFunc(GL_GREATER); }

void renderer::set_depth_eq() { glDepthFunc(GL_EQUAL); }

void renderer::enable_feature(Feature feature) { glEnable(feature); }

void renderer::disable_feature(Feature feature) { glDisable(feature); }

void renderer::begin_frame() {
  glfwPollEvents();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

bool renderer::end_frame() {
  glfwSwapBuffers(window);
  return glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
         glfwWindowShouldClose(window) == 0;
}

renderer::Rect renderer::get_viewport() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  return {0, 0, static_cast<float>(width), static_cast<float>(height)};
}
