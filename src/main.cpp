#define _USE_MATH_DEFINES

#include "applesauce/App.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
#include "applesauce/Camera.h"
#include "applesauce/Mesh.h"

#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <list>
#include <cmath>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

static constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

class Triangles : public App,
                  public Window::ScrollHandler,
                  public Window::MouseHandler,
                  public Window::KeyHandler
{
public:
    struct Entity
    {
        glm::vec3 position;
        float angle;
        size_t meshIndex;
        glm::vec3 color;
    };

public:
    void onKeyDown(int) override
    {
    }

    void onKeyUp(int) override
    {
    }

    void onScroll(double, double yoffset) override
    {
        dist -= yoffset;
        dist = std::max(dist, 0.1f);
    }

    void onMouseDown(MouseHandler::Button button) override
    {
        if (button == MouseHandler::Button::middle)
        {
            move_camera = true;
        }
    }

    void onMouseUp(MouseHandler::Button button) override
    {
        if (button == MouseHandler::Button::middle)
        {
            move_camera = false;
        }
    }

    void onMouseMove(double xpos, double ypos) override
    {
        if (move_camera)
        {
            theta -= (last_xpos - xpos) * 0.1;
            pitch -= (last_ypos - ypos) * 0.1;
        }
        last_xpos = xpos;
        last_ypos = ypos;
    }

    void init() override
    {
        window.setScrollHandler(this);
        window.setMouseHandler(this);
        window.setKeyHandler(this);

        // Loads from assets/shaders/basic.fs.glsl and assets/shaders/basic.vs.glsl
        shader = loadShader("basic");
        shadow = loadShader("shadow");

        const auto levelMeshes = loadMeshes("assets/gltf/wall-and-floor.gltf");

        meshGroups.resize(1);
        meshGroups[0].push_back(levelMeshes.at("Floor"));

        entities.push_back({{0, 0, 0}, 0, 0, glm::vec3{0.5, 0.5, 0.5}});

        camera.fieldOfVision = 42.0f;

        // Init shadow mapping bits

        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void update() override
    {
    }

    void display() override
    {
        glm::vec3 lightDir = glm::normalize(glm::vec3{0.5, 1, 0.25});

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        static float lightDist = 11.2;
        static float lightSize = 18.0f;
        shadow->use();
        glm::mat4 lightSpaceMatrix;
        { // Shadow map part
            glm::mat4 view = glm::lookAt(lightDir * lightDist,
                                         glm::vec3(0),
                                         glm::vec3(0, 1, 0));
            glm::mat4 projection = glm::ortho(-lightSize, lightSize, -lightSize, lightSize, 0.1f, 20.0f);
            lightSpaceMatrix = projection * view;

            for (const auto &entity : entities)
            {
                auto model = glm::mat4(1.0);
                model = glm::translate(model, entity.position);
                model = glm::rotate(model, entity.angle, glm::vec3(0, 1, 0));

                glm::mat4 MVPMatrix = lightSpaceMatrix * model;

                shadow->set("MVPMatrix", MVPMatrix);

                for (const auto &mesh : meshGroups[entity.meshIndex])
                {
                    for (const auto &submesh : mesh.submeshes)
                    {
                        submesh.array->bind();
                        submesh.indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                        glDrawElements(GL_TRIANGLES, submesh.elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(submesh.indexBufferByteOffset));
                    }
                }
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shader->use();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);

        const auto [width, height] = window.framebufferSize();
        camera.viewport = {width, height};
        glViewport(0, 0, width, height);

        window.clear({0.1f, 0.1f, 0.1f, 1.0f});

        camera.position = glm::mat3(glm::yawPitchRoll(theta, pitch, 0.0f)) * glm::vec3{0, 0, -dist};
        glm::mat4 view = camera.lookAtMatrix(glm::vec3{0});
        glm::mat4 projection = camera.projectionMatrix();

        glm::vec3 LightDirection = glm::mat3(view) * lightDir;

        for (const auto &entity : entities)
        {
            auto model = glm::mat4(1.0);
            model = glm::translate(model, entity.position);
            model = glm::rotate(model, entity.angle, glm::vec3(0, 1, 0));

            glm::mat4 modelView = view * model;
            glm::mat3 normalMatrix = glm::mat3(modelView);

            glm::mat4 MVPMatrix = projection * modelView;
            glm::mat4 LightViewMatrix = lightSpaceMatrix * model;

            shader->set("MVPMatrix", MVPMatrix);
            shader->set("ModelViewMatrix", modelView);
            shader->set("LightViewMatrix", LightViewMatrix);
            shader->set("NormalMatrix", normalMatrix);
            shader->set("Color", entity.color);
            shader->set("Ambient", ambient);
            shader->set("LightColor", glm::vec3{1.0, 1.0, 1.0});
            shader->set("LightDirection", LightDirection);
            shader->set("SpecularPower", specularPower);
            shader->set("SpecularStrength", specularStrength);

            for (const auto &mesh : meshGroups[entity.meshIndex])
            {
                for (const auto &submesh : mesh.submeshes)
                {
                    submesh.array->bind();
                    submesh.indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                    glDrawElements(GL_TRIANGLES, submesh.elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(submesh.indexBufferByteOffset));
                }
            }
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMap);
    }

    void cleanUp() override
    {
        std::cout << "Camera Stats:\n";
        std::cout << "\tPitch: " << pitch << std::endl;
        std::cout << "\tTheta: " << theta << std::endl;
        std::cout << "\tDist: " << dist << std::endl;
    }

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> shadow;

    std::vector<Entity> entities;

    std::vector<std::vector<Mesh>> meshGroups;

    Camera camera;
    float pitch = 0.955591;
    float theta = 2.90973;
    float dist = 10.1983;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;

    glm::vec3 ambient{87.0f / 255.0f, 57.0 / 255.0f, 129.0f / 255.0f};

    float specularPower = 32.0f;
    float specularStrength = 1.0f;

    // Shadow map bits
    GLuint depthMapFBO;
    GLuint depthMap;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}