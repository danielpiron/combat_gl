#include "VertexArray.h"
#include "AltMesh.h"

#include <util/gltf.h>

#include <fstream>
#include <string>

static std::string readFileText(const char *filename)
{
    std::ifstream f{filename};
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

namespace applesauce
{
    static VertexAttribute vertexAttribFromName(const std::string &name)
    {
        if (name == "POSITION")
            return VertexAttribute::position;
        else if (name == "NORMAL")
            return VertexAttribute::normal;
        else if (name == "TEXCOORD_0")
            return VertexAttribute::texcoord;
        else
            return VertexAttribute::none;
    }

    std::unordered_map<std::string, Mesh> loadMeshes(const char *filename)
    {
        std::unordered_map<std::string, Mesh> result;

        std::string gltfText(readFileText(filename));
        const auto gltf = glTFFromString(gltfText.c_str());

        std::vector<std::shared_ptr<applesauce::Buffer>> buffers;
        // Load up buffers, these are going directly into OpenGL, which is arguable
        for (const auto &gltfBuffer : gltf.buffers)
        {
            buffers.emplace_back(std::make_shared<applesauce::Buffer>(&gltfBuffer.getBytes()[0], gltfBuffer.byteLength));
        }

        for (const auto &gltfMesh : gltf.meshes)
        {
            std::list<Mesh::Primitive> primitives;
            for (const auto &gltfMeshPrimitive : gltfMesh.primitives)
            {
                auto vertexArray = std::make_shared<applesauce::VertexArray>();
                vertexArray->bind();
                for (const auto &[accessorName, accessorIndex] : gltfMeshPrimitive.attributes)
                {
                    const auto &accessor = gltf.accessors[accessorIndex];
                    const auto &bufferView = gltf.bufferViews[accessor.bufferView];
                    const auto offset = bufferView.byteOffset + accessor.byteOffset;
                    const auto vAttrib = vertexAttribFromName(accessorName);

                    applesauce::VertexBufferDescription desc{
                        {
                            vAttrib,
                            accessor.componentCount(),
                            offset,
                            bufferView.byteStride,
                        },
                    };
                    vertexArray->addVertexBuffer(*buffers[bufferView.buffer], desc);
                }

                const auto &indicesAccessor = gltf.accessors[gltfMeshPrimitive.indices];
                const auto &indicesBufferView = gltf.bufferViews[indicesAccessor.bufferView];

                auto indexBuffer = std::make_shared<Buffer>(&gltf.buffers[indicesBufferView.buffer].getBytes()[indicesBufferView.byteOffset + indicesAccessor.byteOffset],
                                                            indicesAccessor.count * 2, Buffer::Target::element_array);

                // Snag just the base color from the material
                auto materialColor = gltf.materials[gltfMeshPrimitive.material].pbrMetallicRoughness.baseColorFactor;
                glm::vec3 baseColor{materialColor[0], materialColor[1], materialColor[2]};
                primitives.emplace_back(Mesh::Primitive{
                    std::make_shared<Material>(Material{baseColor}),
                    vertexArray,
                    indexBuffer,
                    indicesAccessor.count,
                });
                vertexArray->unbind();
            }
            result.emplace(gltfMesh.name, Mesh{primitives});
        }
        return result;
    }
}