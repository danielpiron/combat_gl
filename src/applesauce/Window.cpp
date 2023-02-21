#include "Window.h"

#include <cassert>

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
        assert(false);
    }
}
#endif

Window::Window(int width, int height)
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
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);

    window = glfwCreateWindow(width, height, "", nullptr, nullptr);
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

Window::~Window()
{
    if (window != nullptr)
    {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

void Window::show()
{
    glfwShowWindow(window);
}

void Window::hide()
{
    glfwHideWindow(window);
}

void Window::close()
{
    glfwSetWindowShouldClose(window, GL_TRUE);
}

void Window::setTitle(const char *title)
{
    glfwSetWindowTitle(window, title);
}

// TODO: Needs to move to Renderer
void Window::clear(const glm::vec4 &color) const
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::swapBuffers() const
{
    glfwSwapBuffers(window);
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() const
{
    glfwPollEvents();
}

//
// Event handler specification
//
void Window::setScrollHandler(ScrollHandler *handler)
{
    scrollHandler = handler;
}

void Window::setMouseHandler(MouseHandler *handler)
{
    mouseHandler = handler;
}

void Window::setKeyHandler(KeyHandler *handler)
{
    keyHandler = handler;
}

//
// Event dispatch
//
void Window::dispatchScrollEvent(double xoffset, double yoffset)
{
    if (scrollHandler == nullptr)
        return;
    scrollHandler->onScroll(xoffset, yoffset);
}

void Window::dispatchMouseDownEvent(MouseHandler::Button button)
{
    if (mouseHandler == nullptr)
        return;
    mouseHandler->onMouseDown(button);
}

void Window::dispatchMouseUpEvent(MouseHandler::Button button)
{
    if (mouseHandler == nullptr)
        return;
    mouseHandler->onMouseUp(button);
}

void Window::dispatchMouseMoveEvent(double xpos, double ypos)
{
    if (mouseHandler == nullptr)
        return;
    mouseHandler->onMouseMove(xpos, ypos);
}

void Window::dispatchKeyDownEvent(int key)
{
    if (keyHandler == nullptr)
        return;
    keyHandler->onKeyDown(key);
}

void Window::dispatchKeyUpEvent(int key)
{
    if (keyHandler == nullptr)
        return;
    keyHandler->onKeyUp(key);
}