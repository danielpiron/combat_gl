#pragma once

#include <glm/vec3.hpp>

#include <list>
#include <memory>

namespace applesauce
{
    class VertexArray;
    class Buffer;

    struct Material
    {
        glm::vec3 baseColor;
    };

    struct Mesh
    {
        struct Primitive
        {
            std::shared_ptr<Material> material;
            std::shared_ptr<VertexArray> vertexArray;
            std::shared_ptr<Buffer> indexBuffer;
            int elementCount;
        };
        std::list<Primitive> primitives;
    };

}

// 1. Update makePlaneMesh and makeBoxMesh to return meshes with single primatives.
// 2. Primatives should come with at least one color (or a reference to some material
//    structure)
// 3. Return map of meshes from gltf loader as before
// 4. Consider how to incorporate nodes

applesauce::Mesh makePlaneMesh(float planeSize);
applesauce::Mesh makeBoxMesh(float boxSize);
