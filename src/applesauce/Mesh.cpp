#include "Mesh.h"

#include <util/gltf.h>

#include <fstream>
#include <string>

static std::string readFileText(const char *filename)
{
    std::ifstream f{filename};
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

static applesauce::VertexAttribute vertexAttribFromName(const std::string &name)
{
    if (name == "POSITION")
        return applesauce::VertexAttribute::position;
    else if (name == "NORMAL")
        return applesauce::VertexAttribute::normal;
    else if (name == "TEXCOORD_0")
        return applesauce::VertexAttribute::texcoord;
    else
        return applesauce::VertexAttribute::none;
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
        std::list<SubMesh> submeshes;
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

            submeshes.emplace_back(SubMesh{
                vertexArray,
                buffers,
                buffers[indicesBufferView.buffer],
                indicesBufferView.byteOffset + indicesAccessor.byteOffset,
                indicesAccessor.count,
            });
            vertexArray->unbind();
        }
        result.emplace(gltfMesh.name, Mesh{submeshes});
    }

    return result;
}
