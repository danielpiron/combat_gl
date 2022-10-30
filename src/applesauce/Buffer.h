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