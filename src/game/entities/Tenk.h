#pragma once

#include <applesauce/Input.h>
#include <applesauce/Entity.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

class Shell : public applesauce::Entity
{
    int timer;

    void init(applesauce::ResourceManager& rm)
    {
        mesh = rm.getMesh("TinyBox");
        texture = rm.getTexture("White Square");
        timer = 1000;
    }
    void update(float dt)
    {
        if (timer == 0)
        {
            destroy();
        }
        velocity += glm::vec3(0, -9.8f, 0) * dt;
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

public:
    glm::vec3 velocity;
};


class Tenk : public applesauce::Entity
{
    void init(applesauce::ResourceManager& rm)
    {
        mesh = rm.getMesh("Tenk");
        texture = rm.getTexture("White");
    }
    void update(float dt)
    {
        float spinSpeed = 0;
        float speed = 6.0f;
        if (applesauce::Input::isPressed(GLFW_KEY_A))
        {
            spinSpeed = 4.0f;
        }
        if (applesauce::Input::isPressed(GLFW_KEY_D))
        {
            spinSpeed = -4.0f;
        }
        if (applesauce::Input::isPressed(GLFW_KEY_W))
        {
            glm::vec3 direction = glm::mat3(orientation) * glm::vec3{ 0, 0, -1.0f };
            position += direction * speed * dt;
        }
        if (applesauce::Input::wasJustPressed(GLFW_KEY_SPACE))
        {
            auto shell = std::dynamic_pointer_cast<Shell>(world->spawn(new Shell, position + glm::vec3{ 0, 0.75, 0 }));
            if (shell)
            {
                glm::vec3 direction = glm::mat3(orientation) * glm::vec3{ 0, 0, -1.0f };
                shell->velocity = direction * 20.0f;
            }
        }

        orientation = glm::rotate(orientation, spinSpeed * dt, glm::vec3{ 0, 1.0f, 0 });
    }
};
