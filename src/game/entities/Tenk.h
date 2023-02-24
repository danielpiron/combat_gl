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

    void onTouch(applesauce::Entity &) override
    {
        destroy();
    }

public:
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
        spinOutTimer = 0.0f;
    }
    void update(float dt) override
    {
        float spinSpeed = 0;
        if (spinOutTimer > 0)
        {
            spinSpeed = 40.0f * spinOutTimer * spinOutTimer;
            spinOutTimer -= dt;
        }
        else
        {
            velocity = glm::vec3{0};
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
                velocity = direction * speed;
            }
        }
        if (applesauce::Input::wasJustPressed(shootKey))
        {
            glm::vec3 barrelExit{8.881790563464165e-06f, 0.9173035621643066f, -0.6668300032615662f};
            auto worldBarrelExit = glm::mat3(orientation) * barrelExit + position;

            auto shell = std::dynamic_pointer_cast<Shell>(world->spawn(new Shell, worldBarrelExit));
            if (shell)
            {
                glm::vec3 direction = glm::mat3(orientation) * glm::vec3{0, 0, -1.0f};
                shell->velocity = direction * 20.0f;
                shell->originator = this;
            }
        }
        position += velocity * dt;
        orientation = glm::rotate(orientation, spinSpeed * dt, glm::vec3{0, 1.0f, 0});
    }

    void onTouch() override
    {
        if (spinOutTimer > 0)
        {
            velocity = glm::vec3{0};
        }
    }

    // TODO: We should probably be receiving a smart pointer to the entity
    // One way or another, we need a way to determine that the thing is a
    // shell. For now, we'll go with originator != nullptr
    void onTouch(applesauce::Entity &e) override
    {
        if (e.originator != nullptr)
        {
            velocity = glm::normalize(e.velocity) * 20.0f;
            spinOutTimer = 1.0f;
        }
    }

private:
    int leftKey = GLFW_KEY_A;
    int rightKey = GLFW_KEY_D;
    int forwardKey = GLFW_KEY_W;
    int shootKey = GLFW_KEY_SPACE;
    float spinOutTimer;
};
