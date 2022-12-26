#include "AltMesh.h"

#include "Buffer.h"
#include "VertexArray.h"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <vector>

static applesauce::Mesh::Primitive primitiveFromComponents(const std::vector<glm::vec3> &vertices,
                                                           const std::vector<glm::vec3> &normals,
                                                           const std::vector<glm::vec2> &texcoords,
                                                           const std::vector<uint16_t> &indices)
{
    const int verticesByteCount = sizeof(vertices[0]) * vertices.size();
    const int normalsByteCount = sizeof(normals[0]) * normals.size();
    const int texcoordsByteCount = sizeof(texcoords[0]) * texcoords.size();
    const int indicesByteCount = sizeof(indices[0]) * indices.size();

    auto vertexBuffer = std::make_shared<applesauce::Buffer>(verticesByteCount + normalsByteCount + texcoordsByteCount, applesauce::Buffer::Target::vertex_array);
    auto indexBuffer = std::make_shared<applesauce::Buffer>(indicesByteCount, applesauce::Buffer::Target::element_array);

    { // Set up Vertex Buffer
        vertexBuffer->bind();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(vertexBuffer->map());
        std::memcpy(ptr, &vertices[0], verticesByteCount);
        std::memcpy(ptr + verticesByteCount, &normals[0], normalsByteCount);
        std::memcpy(ptr + verticesByteCount + normalsByteCount, &texcoords[0], texcoordsByteCount);
        vertexBuffer->unmap();
        vertexBuffer->unbind();
    }

    { // Set up IndexBuffer
        indexBuffer->bind();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(indexBuffer->map());
        std::memcpy(ptr, &indices[0], indicesByteCount);
        indexBuffer->unmap();
        indexBuffer->unbind();
    }

    auto vertexArray = std::make_shared<applesauce::VertexArray>();

    applesauce::VertexBufferDescription desc{
        {applesauce::VertexAttribute::position, 3, 0, 0},
        {applesauce::VertexAttribute::normal, 3, verticesByteCount, 0},
        {applesauce::VertexAttribute::texcoord, 2, verticesByteCount + normalsByteCount, 0},
    };

    vertexArray->addVertexBuffer(*vertexBuffer, desc);

    return {nullptr, vertexArray, indexBuffer, static_cast<int>(indices.size())};
}

applesauce::Mesh makePlaneMesh(float planeSize)
{
    // Plane
    //
    //  (-0.5, 0, -0.5)   (0.5, 0, -0.5)
    // -Z          2-----3
    //  ^          | \ B |
    //  |          | A \ |
    //  |          0-----1
    //  (-0.5, 0, 0.5)    (0.5, 0, 0.5)
    //      ----> +X
    //
    // Triangle indicies:
    //   A. 0, 1, 2
    //   B. 1, 3, 2

    float halfSize = planeSize / 2.0f;
    std::vector<glm::vec3> vertices{
        {-halfSize, 0, halfSize},  // 0
        {halfSize, 0, halfSize},   // 1
        {-halfSize, 0, -halfSize}, // 2
        {halfSize, 0, -halfSize},  // 3
    };

    // Normals all face "up"
    std::vector<glm::vec3> normals{
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // and up
    };

    float uvSize = halfSize;
    std::vector<glm::vec2> texcoords{
        {0, uvSize},      // 0 - Near left
        {uvSize, uvSize}, // 1 - Near right
        {0, 0},           // 2 - Far left
        {uvSize, 0},      // 3 - Far Right
    };

    std::vector<uint16_t> indices{
        0, 1, 2, // Triangle A
        1, 3, 2, // Triangle B
    };

    return {{primitiveFromComponents(vertices, normals, texcoords, indices)}};
}

applesauce::Mesh makeBoxMesh(float boxSize)
{
    //
    //
    //         6----------7
    //        /|         /|
    //       / |        / |
    //      3----------2  |
    //      |  |       |  |
    //      |  |       |  |
    //      |  5-------|--4
    //      | /        | /
    //      |/         |/
    //      0----------1
    //
    //

    const float hSize = boxSize / 2.0f;
    std::vector<glm::vec3> corner{
        // Near corners
        {-hSize, -hSize, hSize}, // 0
        {hSize, -hSize, hSize},  // 1
        {hSize, hSize, hSize},   // 2
        {-hSize, hSize, hSize},  // 3
        // Far corners
        {hSize, -hSize, -hSize},  // 4
        {-hSize, -hSize, -hSize}, // 5
        {-hSize, hSize, -hSize},  // 6
        {hSize, hSize, -hSize},   // 7
    };

    std::vector<glm::vec3> sideNormal{
        {0, 0, 1.0f},  // near
        {0, 0, -1.0f}, // far
        {-1.0f, 0, 0}, // left
        {1.0f, 0, 0},  // right
        {0, 1.0f, 0},  // top
        {0, -1.0f, 0}, // bottom
    };

    std::vector<glm::vec3> vertices{
        corner[0], corner[1], corner[2], corner[3], // near    0-3
        corner[4], corner[5], corner[6], corner[7], // far     4-7
        corner[5], corner[0], corner[3], corner[6], // left    8-11
        corner[1], corner[4], corner[7], corner[2], // right   12-15
        corner[3], corner[2], corner[7], corner[6], // top     16-19
        corner[1], corner[0], corner[5], corner[4], // bottom  20-23
    };

    std::vector<glm::vec3> normals{
        sideNormal[0], sideNormal[0], sideNormal[0], sideNormal[0], // near
        sideNormal[1], sideNormal[1], sideNormal[1], sideNormal[1], // far
        sideNormal[2], sideNormal[2], sideNormal[2], sideNormal[2], // left
        sideNormal[3], sideNormal[3], sideNormal[3], sideNormal[3], // right
        sideNormal[4], sideNormal[4], sideNormal[4], sideNormal[4], // top
        sideNormal[5], sideNormal[5], sideNormal[5], sideNormal[5], // bottom
    };

    std::vector<glm::vec2> texcoords{
        // near
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // far
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // left
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // right
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // top
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // bottom
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
    };

    std::vector<uint16_t> indices{
        // near
        0,
        1,
        2,
        0,
        2,
        3,
        // far
        4,
        5,
        6,
        4,
        6,
        7,
        // left
        8,
        9,
        10,
        8,
        10,
        11,
        // right
        12,
        13,
        14,
        12,
        14,
        15,
        // top
        16,
        17,
        18,
        16,
        18,
        19,
        // bottom
        20,
        21,
        22,
        20,
        22,
        23,
    };

    return {{primitiveFromComponents(vertices, normals, texcoords, indices)}};
}