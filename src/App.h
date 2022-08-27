#pragma once

#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#define WIDTH 640
#define HEIGHT 480

#ifdef GLAD_DEBUG
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
        fprintf(stderr, "ERROR %d in %s\n", error_code, name);
    }
}
#endif

class ShaderStage
{
public:
    enum class Type
    {
        fragment,
        vertex
    };

    static GLenum glShaderEnum(const Type type)
    {
        switch (type)
        {
        case Type::fragment:
            return GL_FRAGMENT_SHADER;
        case Type::vertex:
            return GL_VERTEX_SHADER;
        }
    }

public:
    ShaderStage() = default;
    ShaderStage(const std::string &text, const Type type)
        : id(glCreateShader(glShaderEnum(type)))
    {
        const char *source = text.c_str();
        glShaderSource(id, 1, &source, NULL);
    }
    ~ShaderStage()
    {
        if (id != 0)
            glDeleteShader(id);
    }

    ShaderStage &operator=(ShaderStage &&rhs)
    {
        id = rhs.id;
        rhs.id = 0;
        return *this;
    }

    bool compile() const
    {
        glCompileShader(id);

        GLint compile_status;
        glGetShaderiv(id, GL_COMPILE_STATUS, &compile_status);
        return compile_status == GL_TRUE;
    }

    std::string error_log() const
    {
        GLchar buffer[1024];
        glGetShaderInfoLog(id, 1024, NULL, buffer);
        return std::string(buffer);
    }

    GLuint glId() const
    {
        return id;
    }

private:
    GLuint id = 0;
};

class Shader
{
public:
    using Stage = ShaderStage;

public:
    Shader() : id(glCreateProgram()) {}
    ~Shader()
    {
        glDeleteProgram(id);
    }

    void add_vertex_stage(const std::string &source)
    {
        vertex_stage = Stage(source, Stage::Type::vertex);
        glAttachShader(id, vertex_stage.glId());
    }

    void add_fragment_stage(const std::string &source)
    {
        fragment_stage = Stage(source, Stage::Type::fragment);
        glAttachShader(id, fragment_stage.glId());
    }

    bool compile_and_link()
    {
        if (!vertex_stage.compile())
        {
            stage_error_log = std::string("VERTEX SHADER ") + vertex_stage.error_log();
            return false;
        }
        if (!fragment_stage.compile())
        {
            stage_error_log = std::string("FRAGMENT SHADER ") + fragment_stage.error_log();
            return false;
        }

        glLinkProgram(id);

        GLint link_status;
        glGetProgramiv(id, GL_LINK_STATUS, &link_status);

        return link_status == GL_TRUE;
    }

    std::string error_log() const
    {
        if (!stage_error_log.empty())
        {
            return stage_error_log;
        }
        GLchar buffer[1024];
        glGetProgramInfoLog(id, 1024, NULL, buffer);
        return std::string("LINKING ") + std::string(buffer);
    }

    GLuint glId() const
    {
        return id;
    }

private:
    const GLuint id;
    Stage vertex_stage;
    Stage fragment_stage;
    std::string stage_error_log;
};

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

        /*
        #ifdef GLAD_DEBUG
                glad_set_pre_callback(pre_gl_call);
                glad_set_post_callback(post_gl_call);
        #endif
        */
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