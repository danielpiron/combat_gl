#pragma once

#include "Window.h"
#include "Buffer.h"
#include "Shader.h"

#include <map>
#include <memory>
#include <vector>

#define WIDTH 640
#define HEIGHT 480

class App
{
public:
    App(int width = 640, int height = 480) : window(width, height)
    {
    }
    virtual void init() = 0;
    virtual void update(){};
    virtual void display() = 0;
    virtual void cleanUp() {}

    void run()
    {
        _run(false);
    }

    void run_windowless()
    {
        _run(true);
    }

protected:
    void close()
    {
        window.close();
    }

private:
    void _run(bool windowless)
    {
        if (!windowless)
        {
            window.show();
        }
        init();

        glfwSwapInterval(1);

        while (!window.shouldClose())
        {
            window.pollEvents();

            update();
            display();

            window.swapBuffers();
        }

        cleanUp();
    }

protected:
    Window window;
};