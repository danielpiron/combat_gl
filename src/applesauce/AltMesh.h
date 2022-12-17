#pragma once

#include <memory>

namespace applesauce
{
    class VertexArray;
    class Buffer;
}

struct Mesh
{
    std::shared_ptr<applesauce::VertexArray> vertexArray;
    std::shared_ptr<applesauce::Buffer> indexBuffer;
    int elementCount;
};

Mesh makePlaneMesh(float planeSize);
Mesh makeBoxMesh(float boxSize);