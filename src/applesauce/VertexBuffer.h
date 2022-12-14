#pragma once

#include "Buffer.h"

#include <initializer_list>

namespace applesauce
{
    template <typename T>
    class VertexBuffer : public applesauce::Buffer
    {
    public:
        VertexBuffer(size_t elementCount)
            : applesauce::Buffer(elementCount * sizeof(T),
                                 applesauce::Buffer::Target::vertex_array, sizeof(T))
        {
        }

        template <class InputIt>
        VertexBuffer(InputIt first, InputIt last)
            : applesauce::Buffer((last - first) * sizeof(T),
                                 applesauce::Buffer::Target::vertex_array, sizeof(T))
        {
            bind();
            auto ptr = reinterpret_cast<T *>(map());
            for (; first != last; first++)
            {
                *ptr++ = *first;
            }
            unmap();
            unbind();
        }

        VertexBuffer(std::initializer_list<T> init)
            : VertexBuffer(std::begin(init), std::end(init))
        {
        }
    };
}