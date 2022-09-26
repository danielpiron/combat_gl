#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/Buffer.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class AppleSauceBuffer : public AppleSauceTest
{
};

TEST_F(AppleSauceBuffer, CanBeMappedToVertexPointer)
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;

        bool operator==(const Vertex &rhs) const
        {
            return position == rhs.position && texcoord == rhs.texcoord;
        }
    };

    Vertex data[] = {
        {{-1.0, -1.0, 0}, {0, 1.0f}},
        {{1.0, -1.0, 0}, {1.0f, 1.0f}},
        {{-1.0, 1.0, 0}, {1.0f, 0}},
    };

    applesauce::Buffer buffer(sizeof(Vertex) * 3, applesauce::Buffer::Type::vertex);

    buffer.bind();
    auto ptr = reinterpret_cast<Vertex *>(buffer.map());

    ASSERT_TRUE(ptr != nullptr) << "A nullptr indicates a problem with glMapBuffer";

    ptr[0] = data[0];
    ptr[1] = data[1];
    ptr[2] = data[2];

    ASSERT_TRUE(buffer.unmap()) << "False indicates that OpenGL picked up a problem during pointer operations";

    Vertex result[3];
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, buffer.size(), result);
    buffer.unbind();

    EXPECT_EQ(result[0], data[0]);
    EXPECT_EQ(result[1], data[1]);
    EXPECT_EQ(result[2], data[2]);
}

TEST_F(AppleSauceBuffer, CanUnbindToBuffer0)
{
    applesauce::Buffer buffer(4, applesauce::Buffer::Type::vertex);

    buffer.bind();
    GLint bufferId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &bufferId);

    EXPECT_NE(bufferId, 0);

    buffer.unbind();
    GLint unboundBufferId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &unboundBufferId);

    EXPECT_EQ(unboundBufferId, 0);
}

TEST_F(AppleSauceBuffer, CanLeaveBufferUnboundAfterConstructor)
{
    applesauce::Buffer buffer(4, applesauce::Buffer::Type::vertex);

    GLint postConstructId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &postConstructId);
    EXPECT_EQ(postConstructId, 0);
}

TEST_F(AppleSauceBuffer, CanVerifyBuffersAreDistinct)
{
    float expected1 = 1234.5f;
    float expected2 = 12.345f;

    applesauce::Buffer buffer1(sizeof(float), applesauce::Buffer::Type::vertex);
    applesauce::Buffer buffer2(sizeof(float), applesauce::Buffer::Type::vertex);

    buffer1.bind();
    auto ptr1 = reinterpret_cast<float *>(buffer1.map());

    *ptr1 = expected1;

    ASSERT_TRUE(buffer1.unmap());
    buffer1.unbind();

    buffer2.bind();
    auto ptr2 = reinterpret_cast<float *>(buffer2.map());

    *ptr2 = expected2;

    ASSERT_TRUE(buffer2.unmap());
    buffer2.unbind();

    float result1;
    buffer1.bind();
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float), &result1);
    buffer1.unbind();

    float result2;
    buffer2.bind();
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float), &result2);
    buffer2.unbind();

    EXPECT_EQ(expected1, result1);
    EXPECT_EQ(expected2, result2);
}

TEST_F(AppleSauceBuffer, CanOnlyMoveConstructAndAssign)
{
    applesauce::Buffer buffer1(sizeof(float), applesauce::Buffer::Type::vertex);

    buffer1.bind();
    GLint buffer1Id;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer1Id);

    EXPECT_NE(buffer1Id, 0) << "buffer1 should cause a non-zero ID to be bound";

    applesauce::Buffer buffer2(std::move(buffer1));

    buffer2.bind();
    GLint buffer2Id;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer2Id);

    EXPECT_EQ(buffer2Id, buffer1Id) << "When bounding buffer2, we should get the same ID as buffer1 originally had";

    buffer1.bind();
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer1Id);

    EXPECT_EQ(buffer1Id, 0) << "Now, binding buffer1 has the same effect as unbind (setting to 0)";

    applesauce::Buffer buffer3 = std::move(buffer2);

    buffer3.bind();
    GLint buffer3Id;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer3Id);

    EXPECT_EQ(buffer3Id, buffer2Id) << "When bounding buffer3, we should get the same ID as buffer2 got from buffer1";

    buffer2.bind();
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &buffer2Id);

    EXPECT_EQ(buffer2Id, 0) << "Now, binding buffer2 has the same effect as unbind (setting to 0)";
}