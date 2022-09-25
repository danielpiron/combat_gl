#include <gtest/gtest.h>

#include <applesauce/Window.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

class BufferTests : public ::testing::Test
{
public:
    BufferTests() : window(640, 480)
    {
        std::cout << "Creating fixture" << std::endl;
    }
    ~BufferTests()
    {
        std::cout << "Destroying fixture" << std::endl;
    }

protected:
    Window window;
};

TEST_F(BufferTests, bob)
{
}

TEST_F(BufferTests, bob2)
{
}