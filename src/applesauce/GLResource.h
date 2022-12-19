#pragma once

#include <glad/glad.h>

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
}