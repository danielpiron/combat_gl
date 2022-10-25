#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/VertexBuffer.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class AppleSauceVertexBuffer : public AppleSauceTest
{
};

struct TestVertex
{
    glm::vec3 position;
    glm::vec2 texcoord;

    bool operator==(const TestVertex &rhs) const
    {
        return position == rhs.position && texcoord == rhs.texcoord;
    }
};

TEST_F(AppleSauceVertexBuffer, CanBeInitalizerListConstructed)
{
    applesauce::VertexBuffer<TestVertex> vb{
        {{-1.0, -1.0, 0}, {0, 1.0f}},
        {{1.0, -1.0, 0}, {1.0f, 1.0f}},
        {{-1.0, 1.0, 0}, {1.0f, 0}}};

    vb.bind();

    GLint bufferId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bufferId);

    EXPECT_NE(bufferId, 0);

    ASSERT_TRUE(glIsBuffer(bufferId));

    ASSERT_EQ(vb.elementCount(), 3);

    std::vector<TestVertex> expected{
        {{-1.0, -1.0, 0}, {0, 1.0f}},
        {{1.0, -1.0, 0}, {1.0f, 1.0f}},
        {{-1.0, 1.0, 0}, {1.0f, 0}}};
    std::vector<TestVertex> result(vb.elementCount());

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, vb.size(), reinterpret_cast<void *>(&result[0]));

    ASSERT_EQ(expected, result);
}

TEST_F(AppleSauceVertexBuffer, CanBeIteratorConstructed)
{
    std::vector<TestVertex> expected{
        {{-1.0, -1.0, 0}, {0, 1.0f}},
        {{1.0, -1.0, 0}, {1.0f, 1.0f}},
        {{-1.0, 1.0, 0}, {1.0f, 0}}};

    applesauce::VertexBuffer<TestVertex> vb(expected.begin(), expected.end());

    vb.bind();

    GLint bufferId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bufferId);

    EXPECT_NE(bufferId, 0);

    ASSERT_TRUE(glIsBuffer(bufferId));

    ASSERT_EQ(vb.elementCount(), 3);

    std::vector<TestVertex> result(vb.elementCount());

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, vb.size(), reinterpret_cast<void *>(&result[0]));

    ASSERT_EQ(expected, result);
}