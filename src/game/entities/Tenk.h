#pragma once

#include <applesauce/Input.h>
#include <applesauce/Entity.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Shell : public applesauce::Entity
{
    int timer;

    void init(applesauce::ResourceManager &rm) override
    {
        mesh = rm.getMesh("TinyBox");
        timer = 1000;
        collidable = true;
        collisionSize = 0.25;
    }
    void update(float dt) override
    {
        if (timer == 0)
        {
            destroy();
        }
        position += velocity * dt;

        // bounce
        if (position.y < 0.125)
        {
            position.y = 0.126;
            velocity.y *= -0.5f;
            velocity.x += velocity.x * -0.1f;
            velocity.z += velocity.z * -0.1f;
        }
        timer--;
    }

    void onTouch() override
    {
        destroy();
    }

public:
    glm::vec3 velocity;
};

class Tenk : public applesauce::Entity
{
public:
    Tenk(int tenkId)
    {
        if (tenkId == 1)
        {
            leftKey = GLFW_KEY_LEFT;
            rightKey = GLFW_KEY_RIGHT;
            forwardKey = GLFW_KEY_UP;
            shootKey = GLFW_KEY_PERIOD;
        }
    }

    void init(applesauce::ResourceManager &rm) override
    {
        mesh = rm.getMesh("Tenk");
        collidable = true;
        collisionSize = 1.7f;
    }
    void update(float dt) override
    {
        float spinSpeed = 0;
        float speed = 6.0f;
        if (applesauce::Input::isPressed(leftKey))
        {
            spinSpeed = 4.0f;
        }
        if (applesauce::Input::isPressed(rightKey))
        {
            spinSpeed = -4.0f;
        }
        if (applesauce::Input::isPressed(forwardKey))
        {
            glm::vec3 direction = glm::mat3(orientation) * glm::vec3{0, 0, -1.0f};
            position += direction * speed * dt;
        }
        if (applesauce::Input::wasJustPressed(shootKey))
        {
            auto shell = std::dynamic_pointer_cast<Shell>(world->spawn(new Shell, position + glm::vec3{0, 0.75, 0}));
            if (shell)
            {
                glm::vec3 direction = glm::mat3(orientation) * glm::vec3{0, 0, -1.0f};
                shell->velocity = direction * 20.0f;
            }
        }

        orientation = glm::rotate(orientation, spinSpeed * dt, glm::vec3{0, 1.0f, 0});
    }

private:
    int leftKey = GLFW_KEY_A;
    int rightKey = GLFW_KEY_D;
    int forwardKey = GLFW_KEY_W;
    int shootKey = GLFW_KEY_SPACE;
};
