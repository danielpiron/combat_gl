#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#define WIDTH 640
#define HEIGHT 480

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