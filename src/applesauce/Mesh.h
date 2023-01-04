#pragma once

#include <glm/vec3.hpp>

#include "Texture.h"

#include <list>
#include <memory>
#include <unordered_map>

namespace applesauce
{
    class VertexArray;
    class Buffer;

    struct Material
    {
        glm::vec3 baseColor;
        float metallicFactor;
        float roughnessFactor;
        std::shared_ptr<Texture> baseTexture = nullptr;
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

    std::unordered_map<std::string, Mesh> loadMeshes(const char *);

}

// 1. Update makePlaneMesh and makeBoxMesh to return meshes with single primatives.
// 2. Primatives should come with at least one color (or a reference to some material
//    structure)
// 3. Return map of meshes from gltf loader as before
// 4. Consider how to incorporate nodes

applesauce::Mesh makePlaneMesh(float planeSize, const std::shared_ptr<applesauce::Material> material);
applesauce::Mesh makePlaneMesh(float planeWidth, float planeHeight, const std::shared_ptr<applesauce::Material> material);
applesauce::Mesh makeBoxMesh(float boxSize, const std::shared_ptr<applesauce::Material> material);
