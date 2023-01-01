#include <applesauce/Entity.h>

#include <glm/gtx/euler_angles.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

float fRand(float max)
{
    return max * static_cast<float>(rand() % 10000) / 10000.0f;
}


class TinyBlock : public applesauce::Entity
{
    glm::vec3 velocity;
    float timer;

    void init(applesauce::ResourceManager& rm)
    {
        mesh = rm.getMesh("TinyBox");
        texture = rm.getTexture("White Square");
        timer = rand() % 400;

        float speed = fRand(12.0f) + 0.6f;
        glm::vec3 vel{ 0, 1.0f, 0 };
        vel = glm::mat3(glm::yawPitchRoll(fRand(1.0f) - 0.5f, 0.0f, fRand(1.0f) - 0.5f)) * vel;
        velocity = vel * speed;
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
};

class Block : public applesauce::Entity
{
    static constexpr float topSpinSpeed = M_PI * 4;

    float timeLimit;
    float timer = 0;
    void init(applesauce::ResourceManager& rm)
    {
        mesh = rm.getMesh("Box");
        texture = rm.getTexture("White Square");
        timeLimit = fRand(10.0f) + 2.0f;
    }
    void update(float dt)
    {
        if (timer >= timeLimit)
        {
            size_t spawnCount = static_cast<size_t>(rand() % 10) + 10;
            for (size_t i = 0; i <= spawnCount; ++i)
            {
                world->spawn(new TinyBlock, position, glm::quat{});
            }
            destroy();
        }
        const float spinSpeed = (timer / timeLimit) * topSpinSpeed * dt;
        orientation = glm::rotate(orientation, spinSpeed, glm::vec3{ 0, 1.0f, 0 });
        timer += dt;
    }
};

class Floor : public applesauce::Entity
{
    void init(applesauce::ResourceManager& rm)
    {
        mesh = rm.getMesh("Plane");
        texture = rm.getTexture("Checker");
    }
    void update(float)
    {
    }
};
