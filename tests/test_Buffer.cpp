#include <gtest/gtest.h>

#include <applesauce/Window.h>
#include <applesauce/Buffer.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class AppleSauceBuffer : public ::testing::Test
{
public:
    AppleSauceBuffer() : window(640, 480)
    {
    }
    ~AppleSauceBuffer()
    {
    }

protected:
    Window window;
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