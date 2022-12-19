#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/Texture.h>

#include <vector>

class AppleSauceTexture : public AppleSauceTest
{
};

using namespace applesauce;

TEST_F(AppleSauceTexture, CanCreateNewTexture)
{
    GLint boundTex = 0;
    {
        Texture2D tex;

        tex.bind();
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);

        EXPECT_NE(0, boundTex);
        tex.unbind();

        EXPECT_TRUE(glIsTexture(boundTex));
    }
    EXPECT_FALSE(glIsTexture(boundTex));
}

TEST_F(AppleSauceTexture, CanSetFilters)
{
    Texture2D tex;
    tex.setMinFilter(Texture2D::Filter::nearest);
    tex.setMagFilter(Texture2D::Filter::nearest);

    tex.bind();

    GLint minFilter;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    GLint magFilter;
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);

    EXPECT_EQ(GL_NEAREST, minFilter);
    EXPECT_EQ(GL_NEAREST, magFilter);

    tex.setMinFilter(Texture2D::Filter::linear);
    tex.setMagFilter(Texture2D::Filter::linear);

    tex.bind();
    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_LINEAR, minFilter);

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);
    EXPECT_EQ(GL_LINEAR, magFilter);

    tex.setMinFilter(Texture2D::Filter::nearestMipMapNearest);
    tex.bind();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_NEAREST_MIPMAP_NEAREST, minFilter);

    tex.setMinFilter(Texture2D::Filter::nearestMipMapLinear);
    tex.bind();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_NEAREST_MIPMAP_LINEAR, minFilter);

    tex.setMinFilter(Texture2D::Filter::linearMipMapNearest);
    tex.bind();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_LINEAR_MIPMAP_NEAREST, minFilter);

    tex.setMinFilter(Texture2D::Filter::linearMipMapLinear);
    tex.bind();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
    EXPECT_EQ(GL_LINEAR_MIPMAP_LINEAR, minFilter);

    tex.setMagFilter(Texture2D::Filter::linearMipMapLinear);
    tex.bind();

    glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &magFilter);
    EXPECT_EQ(GL_LINEAR, magFilter);
}

TEST_F(AppleSauceTexture, CanCreateNewRGBTexture)
{
    std::vector<uint8_t> textureData{
        0xFF, 0x00, 0x00, 0xFF, // red dot
        0x00, 0xFF, 0x00, 0xFF, // green dot
        0x0F, 0x00, 0xFF, 0xFF, // blue dot
        0xFF, 0xFF, 0x00, 0xFF, // yellow dot
    };

    Texture2D tex(Texture2D::Format::rgba);
    tex.setImage(2, 2, Texture2D::Format::rgba, &textureData[0]);

    tex.bind();
    GLint internalFormat;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);

    EXPECT_EQ(GL_RGBA8, internalFormat);

    std::vector<uint8_t> buffer(textureData.size());

    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, &buffer[0]);

    EXPECT_EQ(textureData, buffer);
}