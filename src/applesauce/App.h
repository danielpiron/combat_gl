#pragma once

#include "Window.h"
#include "Buffer.h"
#include "Shader.h"
#include "Input.h"

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
    virtual void update(float){};
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

        double t = 0.0;
        const double step = 1.0f / 60.0f;

        double currentTime = glfwGetTime();
        double accumulator = 0;

        while (!window.shouldClose())
        {
            double newTime = glfwGetTime();
            double frameTime = newTime - currentTime;
            currentTime = newTime;

            
            accumulator += frameTime;

            while (accumulator >= step) {
                applesauce::Input::beginFrame();
                window.pollEvents();

                update(step);
                accumulator -= step;
                t += step;
            }
            
            display();
            
            window.swapBuffers();
        }

        cleanUp();
    }

protected:
    Window window;
};