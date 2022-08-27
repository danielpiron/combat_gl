#include <gtest/gtest.h>
#include <App.h>

TEST(App, CanRunWindowLess)
{

    class TestApp : public App
    {
        void init() override
        {
        }
        void display() override
        {
            close();
        }
    };

    TestApp testApp;
    testApp.run_windowless();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}