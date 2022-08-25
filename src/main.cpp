#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>

#include <iostream>

#define WIDTH 640
#define HEIGHT 480

static const glm::vec2 vertices[] = {
    {-0.90, -0.90},
    {0.85, -0.90},
    {-0.90, 0.85},
    {0.90, -0.85},
    {0.90, 0.90},
    {-0.85, 0.90},
};

void error_callback(int error, const char *description)
{
    std::cerr << "Error (" << error << "): " << description << std::endl;
}

void key_callback(GLFWwindow *window, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

GLuint prepare_shader()
{
    const char *vertex_shader_text = R"(
        #version 410 core

        layout( location = 0 ) in vec4 vPosition;

        void
        main()
        {
            gl_Position = vPosition;
        }
    )";

    const char *fragment_shader_text = R"(
        #version 410 core

        out vec4 fColor;

        void main()
        {
            fColor = vec4(0.5, 0.4, 0.8, 1.0);
        }
    )";

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    {
        GLchar buffer[1024];
        glGetShaderInfoLog(vertex_shader, 1024, NULL, buffer);
        std::cout << "Vertex Shader: " << buffer << std::endl;
    }

    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    {
        GLchar buffer[1024];
        glGetShaderInfoLog(fragment_shader, 1024, NULL, buffer);
        std::cout << "Fragment Shader: " << buffer << std::endl;
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    {
        GLchar buffer[1024];
        glGetProgramInfoLog(program, 1024, NULL, buffer);
        std::cout << "Program: " << buffer << std::endl;
    }
    return program;
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "glfwInit failed" << std::endl;
        return 1;
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Combat GL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "glfwCreateWindow failed" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        return 1;
    }

    const auto program = prepare_shader();
    glUseProgram(program);

    GLuint buf_triangles = 0;
    glGenBuffers(1, &buf_triangles);
    glBindBuffer(GL_ARRAY_BUFFER, buf_triangles);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint vao_triangles = 0;
    glGenVertexArrays(1, &vao_triangles);
    glBindVertexArray(vao_triangles);
    glBindBuffer(GL_ARRAY_BUFFER, buf_triangles);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao_triangles);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0]));

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}