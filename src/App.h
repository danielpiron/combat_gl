#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Buffer.h"
#include "Shader.h"

#include <glm/vec4.hpp>

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#define WIDTH 640
#define HEIGHT 480

#ifdef GLAD_DEBUG
const char *glErrorName(GLenum error)
{
    switch (error)
    {
    case GL_INVALID_ENUM:
        return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "GL_STACK_OVERFLOW";

    case GL_STACK_UNDERFLOW:
        return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "GL_OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "GL_INVALID_FRAMEBUFFER_OPERATION";
    case GL_CONTEXT_LOST:
        return "GL_CONTEXT_LOST";
        /*
    case GL_TABLE_TOO_LARGE:
        return "GL_TABLE_TOO_LARGE";
        */
    default:
        return "UNKNOWN OPENGL ERROR";
    }
}

void pre_gl_call(const char *name, void *, int len_args, ...)
{
    printf("Calling: %s (%d arguments)\n", name, len_args);
}

void post_gl_call(const char *name, void *, int, ...)
{
    GLenum error_code;
    error_code = glad_glGetError();

    if (error_code != GL_NO_ERROR)
    {
        fprintf(stderr, "ERROR %s in %s\n", glErrorName(error_code), name);
    }
}
#endif

class Renderer
{
private:
    static void error_callback(int error, const char *description)
    {
        std::cerr << "Error (" << error << "): " << description << std::endl;
    }

    static void key_callback(GLFWwindow *window, int key, int, int action, int)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }

public:
    Renderer()
    {
        if (!glfwInit())
        {
            throw std::runtime_error("glfwInit failed");
        }

        glfwSetErrorCallback(error_callback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Combat GL", nullptr, nullptr);
        if (window == nullptr)
        {
            throw std::runtime_error("glfwCreateWindow failed");
        }

        glfwMakeContextCurrent(window);

        glfwSetKeyCallback(window, key_callback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("Failed to initialize OpenGL context");
        }

#ifdef GLAD_DEBUG
        // glad_set_pre_callback(pre_gl_call);
        glad_set_post_callback(post_gl_call);
#endif
    }

    ~Renderer()
    {
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    void show_window()
    {
        glfwShowWindow(window);
    }

    auto framebuffer_size() const
    {
        struct ScreenDimensions
        {
            int width;
            int height;
        };
        ScreenDimensions sd;
        glfwGetFramebufferSize(window, &sd.width, &sd.height);
        return sd;
    }

    bool should_close() const
    {
        return glfwWindowShouldClose(window);
    }

    void poll_events() const
    {
        glfwPollEvents();
    }

    void clear(const glm::vec4 &color) const
    {
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    void swap_buffers() const
    {
        glfwSwapBuffers(window);
    }

    void close() const
    {

        glfwSetWindowShouldClose(window, GL_TRUE);
    }

private:
    GLFWwindow *window = nullptr;
};

class App
{
public:
    virtual void init() = 0;
    virtual void display() = 0;

    void run()
    {
        _run(false);
    }

    void run_windowless()
    {
        _run(true);
    }

protected:
    void close()
    {
        renderer.close();
    }

private:
    void _run(bool windowless)
    {
        if (!windowless)
        {
            renderer.show_window();
        }
        init();

        while (!renderer.should_close())
        {
            renderer.poll_events();

            display();

            renderer.swap_buffers();
        }
    }

protected:
    Renderer renderer;
};