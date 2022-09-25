#pragma once

#include "GLResource.h"

#include <initializer_list>
#include <vector>

namespace applesauce
{
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
        Buffer(size_t size, Type type) : GLResource(genGlBuffer()), _target(getGlTarget(type)), _size(size)
        {
            bind();
            glBufferData(_target, size, nullptr, GL_STATIC_DRAW);
            unbind();
        }

        void bind()
        {
            glBindBuffer(_target, id);
        }

        void unbind()
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

        size_t size()
        {
            return _size;
        }

    private:
        const GLenum _target;
        const size_t _size;
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