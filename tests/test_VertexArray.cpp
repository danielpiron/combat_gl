
#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/VertexArray.h>
#include <applesauce/VertexBuffer.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <map>
#include <vector>

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

TEST_F(AppleSauceVertexArray, CanSpecifyVertexAttibutes)
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texcoord;
    };

    applesauce::VertexArray va;
    applesauce::VertexBuffer<Vertex> vb(3);

    vb.bind();
    // Grab the ID of this buffer for later.
    GLint vertexBufferId;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &vertexBufferId);
    vb.unbind();

    applesauce::VertexBufferDescription desc{
        {applesauce::VertexAttribute::position, 3, offsetof(Vertex, position), sizeof(Vertex)},
        {applesauce::VertexAttribute::texcoord, 2, offsetof(Vertex, texcoord), sizeof(Vertex)},
    };

    va.addVertexBuffer(vb, desc);

    va.bind();

    { // Check position attributes
        GLint bufferId;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &bufferId);

        GLint enabled;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

        GLint stride;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

        GLint size;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);

        std::uintptr_t pointer;
        glGetVertexAttribPointerv(0, GL_VERTEX_ATTRIB_ARRAY_POINTER, reinterpret_cast<void **>(&pointer));

        EXPECT_EQ(bufferId, vertexBufferId);
        EXPECT_TRUE(enabled);
        EXPECT_EQ(stride, 20);
        EXPECT_EQ(size, 3);
        EXPECT_EQ(pointer, 0);
    }

    { // Check texcoord attributes
        GLint bufferId;
        glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &bufferId);

        GLint enabled;
        glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);

        GLint stride;
        glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);

        GLint size;
        glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);

        std::uintptr_t pointer;
        glGetVertexAttribPointerv(2, GL_VERTEX_ATTRIB_ARRAY_POINTER, reinterpret_cast<void **>(&pointer));

        EXPECT_EQ(bufferId, vertexBufferId);
        EXPECT_TRUE(enabled);
        EXPECT_EQ(stride, 20);
        EXPECT_EQ(size, 2);
        EXPECT_EQ(pointer, 12);
    }
}