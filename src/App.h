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

class Window
{
public:
    class ScrollHandler
    {
    public:
        virtual void onScroll(double xoffset, double yoffset) = 0;
    };

    class MouseHandler
    {
    public:
        enum class Button
        {
            none,
            left,
            right,
            middle
        };
        virtual void onMouseDown(Button) = 0;
        virtual void onMouseUp(Button) = 0;
        virtual void onMouseMove(double, double) = 0;
    };

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

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        auto userWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        userWindow->dispatchScrollEvent(xoffset, yoffset);
    }

    static MouseHandler::Button translateGLFWButton(int button)
    {
        switch (button)
        {
        case GLFW_MOUSE_BUTTON_LEFT:
            return MouseHandler::Button::left;
        case GLFW_MOUSE_BUTTON_RIGHT:
            return MouseHandler::Button::right;
        case GLFW_MOUSE_BUTTON_MIDDLE:
            return MouseHandler::Button::middle;
        default:
            return MouseHandler::Button::none;
        }
    }

    static void mouse_button_callback(GLFWwindow *window, int button, int action, int)
    {
        auto userWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        switch (action)
        {
        case GLFW_PRESS:
            userWindow->dispatchMouseDownEvent(translateGLFWButton(button));
            break;
        case GLFW_RELEASE:
            userWindow->dispatchMouseUpEvent(translateGLFWButton(button));
            break;
        default:
            break;
        }
    }
    static void mouse_position_callback(GLFWwindow *window, double xpos, double ypos)
    {
        auto userWindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
        userWindow->dispatchMouseMoveEvent(xpos, ypos);
    }

public:
    Window()
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

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, key_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, mouse_position_callback);

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
        {
            throw std::runtime_error("Failed to initialize OpenGL context");
        }

#ifdef GLAD_DEBUG
        // glad_set_pre_callback(pre_gl_call);
        glad_set_post_callback(post_gl_call);
#endif
    }

    ~Window()
    {
        if (window != nullptr)
        {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

    void setScrollHandler(ScrollHandler *handler)
    {
        scrollHandler = handler;
    }

    void setMouseHandler(MouseHandler *handler)
    {
        mouseHandler = handler;
    }

    void show()
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

    bool shouldClose() const
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void swap_buffers() const
    {
        glfwSwapBuffers(window);
    }

    void close() const
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Event handler dispatch
    void dispatchScrollEvent(double xoffset, double yoffset)
    {
        if (scrollHandler == nullptr)
            return;
        scrollHandler->onScroll(xoffset, yoffset);
    }

    void dispatchMouseDownEvent(MouseHandler::Button button)
    {
        if (mouseHandler == nullptr)
            return;
        mouseHandler->onMouseDown(button);
    }

    void dispatchMouseUpEvent(MouseHandler::Button button)
    {
        if (mouseHandler == nullptr)
            return;
        mouseHandler->onMouseUp(button);
    }

    void dispatchMouseMoveEvent(double xpos, double ypos)
    {
        if (mouseHandler == nullptr)
            return;
        mouseHandler->onMouseMove(xpos, ypos);
    }

private:
    GLFWwindow *window = nullptr;

    ScrollHandler *scrollHandler = nullptr;
    MouseHandler *mouseHandler = nullptr;
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
        window.close();
    }

private:
    void _run(bool windowless)
    {
        if (!windowless)
        {
            window.show();
        }
        init();

        while (!window.shouldClose())
        {
            window.poll_events();

            display();

            window.swap_buffers();
        }
    }

protected:
    Window window;
};