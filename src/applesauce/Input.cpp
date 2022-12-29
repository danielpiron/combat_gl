#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cstring>

namespace applesauce
{
    namespace Input
    {
        struct KeyChange
        {
            unsigned int justPressed : 1, justReleased : 1;
        };

        static bool keyStatus[GLFW_KEY_LAST + 1];
        static KeyChange keyChange[GLFW_KEY_LAST + 1];

        void init()
        {
            memset(keyStatus, 0, sizeof(keyStatus));
            memset(keyChange, 0, sizeof(keyChange));
        }

        void beginFrame()
        {
            memset(keyChange, 0, sizeof(keyChange));
        }

        void press(int key)
        {
            keyStatus[key] = 1;
            keyChange[key].justPressed = 1;
        }

        void release(int key)
        {
            keyStatus[key] = 0;
            keyChange[key].justReleased = 1;
        }

        bool isPressed(int key)
        {
            return keyStatus[key];
        }

        bool wasJustPressed(int key)
        {
            return keyChange[key].justPressed;
        }

        bool wasJustReleased(int key)
        {
            return keyChange[key].justReleased;
        }
    }
}