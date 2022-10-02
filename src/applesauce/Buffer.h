#pragma once

#include "GLResource.h"

#include <initializer_list>
#include <vector>

namespace applesauce
{
    class GLResource
    {
    public:
        GLResource() : id(0) {}
        GLResource(const GLuint resourceId) : id(resourceId) {}

        GLResource(const GLResource &) = delete;
        GLResource(GLResource &&other) : GLResource(other.id)
        {
            other.id = 0;
        }
        GLResource &operator=(const GLResource &) = delete;
        GLResource &operator=(GLResource &&other)
        {
            id = other.id;
            other.id = 0;
            return *this;
        }

    protected:
        GLuint glId() const
        {
            return id;
        }

    private:
        GLuint id;
    };

    class Buffer : public GLResource
    {
    public:
        enum class Type
        {
            vertex,
        };

    private:
        static GLuint genGlBuffer()
        {
            GLuint new_id;
            glGenBuffers(1, &new_id);
            return new_id;
        }

        static GLenum getGlTarget(Type type)
        {
            switch (type)
            {
            case Type::vertex:
                return GL_ARRAY_BUFFER;
            default:
                return 0;
            }
        }

    public:
        Buffer(size_t size, Type type, size_t elementSize = 1)
            : GLResource(genGlBuffer()), _target(getGlTarget(type)), _size(size), _elementSize(elementSize)
        {
            bind();
            glBufferData(_target, size, nullptr, GL_STATIC_DRAW);
            unbind();
        }

        void bind() const
        {
            glBindBuffer(_target, glId());
        }

        void unbind() const
        {
            glBindBuffer(_target, 0);
        }

        void *map()
        {
            return glMapBuffer(_target, GL_WRITE_ONLY);
        }

        bool unmap()
        {
            return glUnmapBuffer(_target);
        }

        size_t size() const
        {
            return _size;
        }

        size_t elementCount() const
        {
            return _size / _elementSize;
        }

    private:
        const GLenum _target;
        const size_t _size;
        const size_t _elementSize;
    };
}

template <typename T>
class Buffer : public GLResource
{
public:
    Buffer(std::initializer_list<T> init) : GLResource(genBuffer()), _size(init.size())
    {
        // Save previous buffer before binding a new one
        GLint currentBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, init.size() * sizeof(T), nullptr, GL_STATIC_DRAW);

        T *ptr = reinterpret_cast<T *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        for (const auto &element : init)
        {
            *ptr++ = element;
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glBindBuffer(GL_ARRAY_BUFFER, currentBuffer);
    }

    ~Buffer()
    {
        if (id != 0)
        {
            glDeleteBuffers(1, &id);
        }
    }

    size_t size() const
    {
        return _size;
    }

private:
    static GLuint genBuffer()
    {
        GLuint new_id;
        glGenBuffers(1, &new_id);
        return new_id;
    }

private:
    size_t _size;
};