#include <gtest/gtest.h>

#include "AppleSauceTest.h"
#include <applesauce/Texture.h>

class AppleSauceTexture : public AppleSauceTest
{
};

TEST_F(AppleSauceTexture, CanCreateNewTexture)
{
    GLint boundTex = 0;
    {
        applesauce::Texture2D tex;

        tex.bind();
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);

        EXPECT_NE(0, boundTex);
        tex.unbind();

        EXPECT_TRUE(glIsTexture(boundTex));
    }
    EXPECT_FALSE(glIsTexture(boundTex));
}