#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <vector>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h> 
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "my_math.h"

struct Vertex
{
  lib::Vec3 pos;
  lib::Vec3 col;
};

static const Vertex vertices[] = {

 { {-1.f, -1.f, -1.f},  {0.f, 0.f, 0.f} }, // 0
 { {-1.f, 1.f, -1.f},   {0.f, 1.f, 0.f} }, // 1
 { {1.f, 1.f, -1.f},    {1.f, 1.f, 0.f} }, // 2
 { {1.f, -1.f, -1.f},   {1.f, 0.f, 0.f} }, // 3
 { {-1.f, -1.f, 1.f},   {0.f, 0.f, 1.f} }, // 4
 { {-1.f, 1.f, 1.f},    {0.f, 1.f, 1.f} }, // 5
 { { 1.f, 1.f, 1.f},    {1.f, 1.f, 1.f} }, // 6
 { { 1.f, -1.f, 1.f},   {1.f, 0.f, 1.f} }  // 7
};

static const GLuint indices[] = {
                          0, 1, 2, 0, 2, 3,
                          4, 6, 5, 4, 7, 6,
                          4, 5, 1, 4, 1, 0,
                          3, 2, 6, 3, 6, 7,
                          1, 5, 6, 1, 6, 2,
                          4, 0, 3, 4, 3, 7
};

static const char* vertex_shader_text =
"#version 410 core\n"
"uniform mat4 Model;\n"
"layout (std140) uniform Matrices\n"
"{\n"
"    mat4 Proj;\n"
"    mat4 View;\n"
"};\n"
"uniform float time;\n"
"layout(location = 0) in vec3 vPos;\n"
"layout(location = 1) in vec3 vCol;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = Proj * View * Model *  vec4(vPos, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 410\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(void)
{
  glfwSetErrorCallback(error_callback);

  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1200, 1200, "OpenGL Cube", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(window, key_callback);

  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
  glfwSwapInterval(1);

  // NOTE: OpenGL error checks have been omitted for brevity



  glEnable(GL_DEPTH_TEST);
  glEnable(GL_FRAMEBUFFER_SRGB); // linear color input and then gamma corrected framebuffer
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  GLuint vertex_buffer;
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
  glCompileShader(vertex_shader);

  const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
  glCompileShader(fragment_shader);

  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);

  const GLint mvp_location = glGetUniformLocation(program, "Model");
  const GLint vpos_location = glGetAttribLocation(program, "vPos");
  const GLint vcol_location = glGetAttribLocation(program, "vCol");

  GLuint vertex_array;
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex), (void*)offsetof(Vertex, pos));
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
    sizeof(Vertex), (void*)offsetof(Vertex, col));

  // obtain location of the uniform block
  GLuint Matrices_binding = 0;
  GLint matricesIndex = glGetUniformBlockIndex(program, "Matrices");
  // Bind the uniform block to the binding point
  glUniformBlockBinding(program, matricesIndex, Matrices_binding);
  // Create uniform buffer object for matrices
  GLuint uboMatrices;
  glGenBuffers(1, &uboMatrices);
  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(lib::Mat4), NULL, GL_STATIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, Matrices_binding, uboMatrices);

  while (!glfwWindowShouldClose(window))
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float time = (float)glfwGetTime();
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    const float ratio = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    lib::Vec3 camera_pos = { 3.0f, 0.0f, 3.0f };
    lib::Vec3 camera_target = { 0.0f, 0.0f, 0.0f, };
    lib::Mat4 view = lib::create_look_at(camera_pos, camera_target, { 0.0f, 1.0f, 0.0f });
    lib::Mat4 translate = lib::create_translate({ -0.33f, 0.0f, 0.0f });
    lib::Mat4 rotation = lib::create_rotation_z((f32)time);
    lib::Mat4 projection = lib::create_perspective(lib::deg_to_rad(50.0f), (f32)width / height, 0.1f, 100.0f);
    lib::Mat4 model = translate * rotation;

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(lib::Mat4), &projection);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(lib::Mat4), sizeof(lib::Mat4), &view);

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)&model);
    glUniform1f(glGetUniformLocation(program, "time"), time);
    glBindVertexArray(vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Bind the EBO
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwDestroyWindow(window);

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
