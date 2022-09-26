
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
        ~VertexArray()
        {
            if (glId() != 0)
            {
                auto id = glId();
                glDeleteVertexArrays(1, &id);
            }
        }

        void bind()
        {
            glBindVertexArray(glId());
        }

        void unbind()
        {
            glBindVertexArray(0);
        }
    };
}

class AppleSauceVertexArray : public AppleSauceTest
{
};

TEST_F(AppleSauceVertexArray, CanCreateNewVertexArray)
{
    GLint boundId;
    {
        applesauce::VertexArray va;
        va.bind();

        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundId);

        EXPECT_NE(boundId, 0) << "boundId is expected to be non-zero";
        EXPECT_TRUE(glIsVertexArray(boundId)) << "OpenGL should recognize boundId as a vertex array";

        va.unbind();
        GLint boundId;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundId);
        EXPECT_EQ(boundId, 0) << "boundId is expected to be zero after unbind";
    }
    EXPECT_FALSE(glIsVertexArray(boundId)) << "Outside of scope, boundId should no longer be a vertex array";
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