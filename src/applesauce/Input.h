#pragma once

namespace applesauce
{
    namespace Input
    {
        void init();
        void beginFrame();
        void press(int key);
        void release(int key);
        bool isPressed(int key);
        bool wasJustPressed(int key);
        bool wasJustReleased(int key);
    }
}