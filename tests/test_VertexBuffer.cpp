
#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/Buffer.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <map>
#include <vector>

namespace applesauce
{
    struct VertexAttributeDescription
    {
        const char *name;
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

        void bind()
        {
            glBindVertexArray(glId());
        }
    };
}

class AppleSauceVertexArray : public AppleSauceTest
{
};

TEST_F(AppleSauceVertexArray, CanCreateNewVertexArray)
{
    applesauce::VertexArray va;
    va.bind();

    GLint boundId;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundId);

    EXPECT_NE(boundId, 0);
    EXPECT_TRUE(glIsVertexArray(boundId));

    // Check "is vertex array" should be false after destruction
}

/*
TEST_F(AppleSauceVertexArray, bob)
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    applesauce::VertexArray va;
    applesauce::Buffer vb(sizeof(Vertex) * 3, applesauce::Buffer::Type::vertex);
    applesauce::VertexBufferDescription desc{
        {"vPosition", 3, 0, sizeof(Vertex)},
        {"vTexCoord", 2, sizeof(glm::vec3), sizeof(Vertex)},
    };
    int offset = 0;

    va.addVertexBuffer(vb, desc, offset);
}
*/