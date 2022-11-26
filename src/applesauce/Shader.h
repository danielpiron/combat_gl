#pragma once

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include <map>
#include <string>
#include <vector>

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
    struct Attribute
    {
        GLint location;
        GLenum type;
        GLsizei size;

        bool operator==(const Attribute &rhs) const
        {
            return location == rhs.location && type == rhs.type && size == rhs.size;
        }
    };

    struct Uniform
    {
        GLint location;
        GLenum type;
        GLsizei size;

        bool operator==(const Uniform &rhs) const
        {
            return location == rhs.location && type == rhs.type && size == rhs.size;
        }
    };

    using Stage = ShaderStage;
    using Attributes = std::map<std::string, Attribute>;
    using Uniforms = std::map<std::string, Uniform>;

public:
    Shader() : id(glCreateProgram())
    {
    }
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

    Attributes attributes() const
    {
        Attributes result;

        std::vector<GLchar> buffer(gl_parameter(GL_ACTIVE_ATTRIBUTE_MAX_LENGTH));
        GLint attribute_count = gl_parameter(GL_ACTIVE_ATTRIBUTES);
        for (decltype(attribute_count) i = 0; i < attribute_count; ++i)
        {
            GLsizei nameLength = 0;
            GLint attribSize = 0;
            GLenum attribType = 0;
            glGetActiveAttrib(id, i, buffer.size(), &nameLength, &attribSize, &attribType, &buffer[0]);

            std::string name(buffer.begin(), buffer.begin() + nameLength);
            GLint attribLocation = glGetAttribLocation(id, name.c_str());

            result.emplace(name, Attribute{attribLocation, attribType, attribSize});
        }
        return result;
    }

    Uniforms uniforms() const
    {
        Uniforms result;

        std::vector<GLchar> buffer(gl_parameter(GL_ACTIVE_UNIFORM_MAX_LENGTH));
        GLint uniform_count = gl_parameter(GL_ACTIVE_UNIFORMS);
        for (decltype(uniform_count) i = 0; i < uniform_count; ++i)
        {
            GLsizei nameLength = 0;
            GLint uniformSize = 0;
            GLenum uniformType = 0;
            glGetActiveUniform(id, i, buffer.size(), &nameLength, &uniformSize, &uniformType, &buffer[0]);

            std::string name(buffer.begin(), buffer.begin() + nameLength);
            GLint uniformLocation = glGetUniformLocation(id, name.c_str());

            result.emplace(name, Uniform{uniformLocation, uniformType, uniformSize});
        }
        return result;
    }

    void set(const char *name, const glm::mat4 &matrix)
    {
        const auto loc = glGetUniformLocation(glId(), name);
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set(const char *name, const glm::mat3 &matrix)
    {
        const auto loc = glGetUniformLocation(glId(), name);
        glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void set(const char *name, const glm::vec3 &vector)
    {
        const auto loc = glGetUniformLocation(glId(), name);
        glUniform3fv(loc, 1, glm::value_ptr(vector));
    }

    void set(const char *name, const glm::vec4 &vector)
    {
        const auto loc = glGetUniformLocation(glId(), name);
        glUniform4fv(loc, 1, glm::value_ptr(vector));
    }

    void use() const
    {
        glUseProgram(glId());
    }

    GLuint glId() const
    {
        return id;
    }

private:
    GLint gl_parameter(GLenum property_name) const
    {
        GLint result;
        glGetProgramiv(id, property_name, &result);
        return result;
    }

private:
    const GLuint id;
    Stage vertex_stage;
    Stage fragment_stage;
    std::string stage_error_log;
};