#pragma once

#include <gtest/gtest.h>
#include <applesauce/Window.h>

class AppleSauceTest : public ::testing::Test
{
public:
    AppleSauceTest() : window(640, 480)
    {
    }
    ~AppleSauceTest()
    {
    }

protected:
    Window window;
};
