#pragma once

#include "Mesh.h"
#include "Texture.h"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>

namespace applesauce
{

    struct Entity;
    struct IWorld
    {
        virtual std::shared_ptr<Entity> spawn(Entity *, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{glm::vec3{0}}) = 0;
    };

    class ResourceManager
    {
    public:
        virtual std::shared_ptr<applesauce::Mesh> getMesh(const std::string &) = 0;
        virtual std::shared_ptr<applesauce::Texture2D> getTexture(const std::string &) = 0;
    };

    struct Entity
    {
        glm::vec3 position = glm::vec3{0};
        glm::quat orientation = glm::quat{};
        std::shared_ptr<Mesh> mesh = nullptr;

        glm::mat4 modelMatrix = glm::mat4{1.0f};

        IWorld *world = nullptr;
        bool collidable = false;
        float collisionSize = 0;
        Entity *originator = nullptr;

        virtual void init(ResourceManager &) {}
        virtual void update(float) {}
        virtual ~Entity() {}
        void destroy()
        {
            isPendingDestruction = true;
        }

        bool isPendingDestruction = false;

        virtual void onTouch() {}
        virtual void onTouch(Entity &) {}
    };

}