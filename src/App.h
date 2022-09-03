#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Buffer.h"
#include "Shader.h"

#include <iostream>
#include <map>
#include <memory>
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

class App
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
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

private:
    void _run(bool windowless)
    {
        if (!glfwInit())
        {
            std::cerr << "glfwInit failed" << std::endl;
            return;
        }

        glfwSetErrorCallback(error_callback);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        if (windowless)
        {
            glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
        }

        window = glfwCreateWindow(WIDTH, HEIGHT, "Combat GL", nullptr, nullptr);
        if (window == nullptr)
        {
            std::cerr << "glfwCreateWindow failed" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        glfwSetKeyCallback(window, key_callback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            std::cout << "Failed to initialize OpenGL context" << std::endl;
            return;
        }

#ifdef GLAD_DEBUG
        // glad_set_pre_callback(pre_gl_call);
        glad_set_post_callback(post_gl_call);
#endif
        init();

        glfwSwapInterval(1);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();

            display();

            glfwSwapBuffers(window);
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }

private:
    GLFWwindow *window;
};