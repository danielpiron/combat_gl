#pragma once

#include "Buffer.h"

#include <initializer_list>

namespace applesauce
{
    template <typename T>
    class VertexBuffer : public applesauce::Buffer
    {
    public:
        VertexBuffer(std::initializer_list<T> init)
            : applesauce::Buffer(init.size() * sizeof(T),
                                 applesauce::Buffer::Type::vertex, sizeof(T))
        {
            bind();
            auto ptr = reinterpret_cast<T *>(map());
            for (auto &element : init)
            {
                *ptr++ = element;
            }
            unmap();
            unbind();
        }
    };
}