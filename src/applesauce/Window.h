#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/vec4.hpp>

#include <iostream>
#include <stdexcept>

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
    Window(int width, int height);
    ~Window();

    void setTitle(const char *);

    void show();
    void hide();
    void close();

    auto framebufferSize() const
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

    bool shouldClose() const;
    void pollEvents() const;
    void swapBuffers() const;

    void clear(const glm::vec4 &color) const;

    // Event handler specification
    void setScrollHandler(ScrollHandler *handler);
    void setMouseHandler(MouseHandler *handler);

    // Event handler dispatch
    void dispatchScrollEvent(double xoffset, double yoffset);
    void dispatchMouseDownEvent(MouseHandler::Button button);
    void dispatchMouseUpEvent(MouseHandler::Button button);
    void dispatchMouseMoveEvent(double xpos, double ypos);

private:
    GLFWwindow *window = nullptr;

    ScrollHandler *scrollHandler = nullptr;
    MouseHandler *mouseHandler = nullptr;
};
