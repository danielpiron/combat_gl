#pragma once

#include "GLResource.h"
#include "Buffer.h"

#include <vector>

namespace applesauce
{
    enum class VertexAttribute
    {
        position = 0,
        normal,
        texcoord,
    };

    struct VertexAttributeDescription
    {
        const VertexAttribute attrib;
        const int size;
        const int offset;
        const int stride;
    };

    using VertexBufferDescription = std::vector<VertexAttributeDescription>;

    class VertexArray : public GLResource
    {
    private:
        static GLuint genGlVertexArray()
        {
            GLuint newId;
            glGenVertexArrays(1, &newId);
            return newId;
        }

    public:
        VertexArray() : GLResource(genGlVertexArray()) {}
        ~VertexArray()
        {
            if (glId() != 0)
            {
                auto id = glId();
                glDeleteVertexArrays(1, &id);
            }
        }

        void bind() const
        {
            glBindVertexArray(glId());
        }

        void unbind() const
        {
            glBindVertexArray(0);
        }

        void addVertexBuffer(const applesauce::Buffer &buffer, const VertexBufferDescription &descriptions)
        {
            bind();
            buffer.bind();

            for (const auto &desc : descriptions)
            {
                const auto index = static_cast<GLuint>(desc.attrib);
                glVertexAttribPointer(index, desc.size, GL_FLOAT, GL_FALSE, desc.stride, reinterpret_cast<void *>(desc.offset));
                glEnableVertexAttribArray(index);
            }

            buffer.unbind();
            unbind();
        }
    };
}