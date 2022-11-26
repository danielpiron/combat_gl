#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct Camera
{
    struct Viewport
    {
        float width;
        float height;

        Viewport(float w, float h) : width(w), height(h) {}
        Viewport(int w, int h) : width(static_cast<float>(w)), height(static_cast<float>(h))
        {
        }
    };

    glm::vec3 position{0};
    glm::vec3 upVector{0, 1, 0};
    Viewport viewport{640, 480};
    float fieldOfVision = 60.0f;
    float nearPlaneDistance = 0.1f;
    float farPlaneDistance = 100.f;

    glm::mat4 lookAtMatrix(const glm::vec3 &target) const
    {
        return glm::lookAt(position, target, upVector);
    }

    glm::mat4 projectionMatrix() const
    {
        return glm::perspectiveFov(glm::radians(fieldOfVision), viewport.width, viewport.height, 0.1f, 100.0f);
    }
};