#pragma once

#include "GLResource.h"

#include <initializer_list>
#include <vector>

template <typename T>
class Buffer : public GLResource
{
public:
    Buffer(std::initializer_list<T> init) : GLResource(genBuffer()), _size(init.size())
    {
        std::vector<T> vec(init);
        // Save previous buffer before binding a new one
        GLint currentBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &currentBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, vec.size() * sizeof(vec[0]), &vec[0], GL_STATIC_DRAW);

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