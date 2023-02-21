#define _USE_MATH_DEFINES

#include "applesauce/App.h"
#include "applesauce/Debug.h"
#include "applesauce/Entity.h"
#include "applesauce/Input.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
#include "applesauce/Texture.h"
#include "applesauce/Camera.h"
#include "applesauce/Mesh.h"

#include "game/entities/Tenk.h"
#include "game/entities/Level.h"
#include "game/entities/TestArea.h"
#include "game/Collision.h"

#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static constexpr unsigned int SHADOW_WIDTH = 2048,
                              SHADOW_HEIGHT = 2048;

Quad quadFromEntity(const applesauce::Entity &entity, float size)
{
    float halfSize = size / 2.0f;
    glm::vec4 upperLeft{-halfSize, 0, -halfSize, 1.0f};
    glm::vec4 lowerLeft{-halfSize, 0, halfSize, 1.0f};
    glm::vec4 lowerRight{halfSize, 0, halfSize, 1.0f};
    glm::vec4 upperRight{halfSize, 0, -halfSize, 1.0f};

    auto tfUpperLeft = entity.modelMatrix * upperLeft;
    auto tfLowerLeft = entity.modelMatrix * lowerLeft;
    auto tfLowerRight = entity.modelMatrix * lowerRight;
    auto tfUpperRight = entity.modelMatrix * upperRight;

    return {glm::vec2{tfUpperLeft.x, tfUpperLeft.z},
            glm::vec2{tfLowerLeft.x, tfLowerLeft.z},
            glm::vec2{tfLowerRight.x, tfLowerRight.z},
            glm::vec2{tfUpperRight.x, tfUpperRight.z}};
}

AABB aabbFromEntity(const applesauce::Entity &entity)
{
    float halfSize = entity.collisionSize / 2.0f;
    return {{entity.position.x - halfSize, entity.position.z - halfSize},
            {entity.position.x + halfSize, entity.position.z + halfSize}};
}

class Triangles : public App,
                  public applesauce::IWorld,
                  public applesauce::ResourceManager,
                  public Window::ScrollHandler,
                  public Window::MouseHandler,
                  public Window::KeyHandler
{
public:
    void onKeyDown(int keycode) override
    {
        std::cout << "KeyDown: " << keycode << std::endl;
        applesauce::Input::press(keycode);
    }

    void onKeyUp(int keycode) override
    {
        std::cout << "KeyUp: " << keycode << std::endl;
        applesauce::Input::release(keycode);
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

    std::shared_ptr<applesauce::Mesh> getMesh(const std::string &key) override
    {
        return meshes[key];
    }
    std::shared_ptr<applesauce::Texture2D> getTexture(const std::string &key) override
    {
        return textures[key];
    }

    void init() override
    {

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsDark();

        const char *glsl_version = "#version 150";

        ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow(), true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        window.setScrollHandler(this);
        window.setMouseHandler(this);
        window.setKeyHandler(this);

        // Loads from assets/shaders/basic.fs.glsl and assets/shaders/basic.vs.glsl
        shader = loadShader("basic");
        shadow = loadShader("shadow");
        quad = loadShader("quad");

        textures.emplace("White", applesauce::singleColorTexture(0xFFFFFFFF));
        textures.emplace("Checker", applesauce::textureFromPNG("assets/textures/Checker.png"));
        textures.emplace("White Square", applesauce::textureFromPNG("assets/textures/White Square.png"));

        auto boxMaterial = std::make_shared<applesauce::Material>(applesauce::Material{{1.0f, 1.0f, 1.0f}, // baseColor - white
                                                                                       0.5,                // roughnessFactor
                                                                                       0.5,                // metallicFactor
                                                                                       getTexture("White Square")});

        auto checkerMaterial = std::make_shared<applesauce::Material>(applesauce::Material{{1.0f, 0.6f, 0.1f}, // baseColor - white
                                                                                           0.5,                // roughnessFactor
                                                                                           0.5,                // metallicFactor
                                                                                           getTexture("Checker")});

        meshes.emplace("TinyBox", std::make_shared<applesauce::Mesh>(makeBoxMesh(0.25f, boxMaterial)));
        meshes.emplace("Box", std::make_shared<applesauce::Mesh>(makeBoxMesh(1.0f, boxMaterial)));

        for (auto &[name, mesh] : applesauce::loadMeshes("assets/gltf/tenk9aa.gltf"))
        {
            for (auto prim : mesh.primitives)
            {
                prim.material->baseTexture = getTexture("White");
            }
            meshes.emplace(name, std::make_shared<applesauce::Mesh>(mesh));
        }

        for (auto &[name, mesh] : applesauce::loadMeshes("assets/gltf/wall-and-floor.gltf"))
        {
            for (auto prim : mesh.primitives)
            {
                prim.material->baseTexture = getTexture("White");
                prim.material->baseColor = glm::vec3{0.3, 0.3, 1.0};
            }
            meshes.emplace(name, std::make_shared<applesauce::Mesh>(mesh));
        }

        const char *playField = "********************************\n"
                                "**                             *\n"
                                "*                              *\n"
                                "*              **              *\n"
                                "*              **              *\n"
                                "*              **              *\n"
                                "*    **                  **    *\n"
                                "*     *                  *     *\n"
                                "*  T  *   ***      ***   *  T  *\n"
                                "*     *   ***      ***   *     *\n"
                                "*     *                  *     *\n"
                                "*    **                  **    *\n"
                                "*              **              *\n"
                                "*              **              *\n"
                                "*              **              *\n"
                                "*                            ***\n"
                                "*                            ***\n"
                                "********************************";

        prepareTileMap(playField, tm);

        std::stringstream stream(playField);
        std::string line;

        int tankId = 0;
        int row = 0;
        int maxCol = -1;
        while (std::getline(stream, line, '\n'))
        {
            int col = 0;
            for (const auto &character : line)
            {
                const glm::vec3 position{col, 0, row};
                switch (character)
                {
                case '*':
                    spawn(new Wall(), position);
                    break;
                case 'T':
                    auto t = spawn(new Tenk(tankId++), position);
                    tenks.push_back(std::dynamic_pointer_cast<Tenk>(t));
                    break;
                }

                col++;
            }
            maxCol = std::max(maxCol, col);
            row++;
        }

        for (auto &entity : entities)
        {
            entity->position.x -= static_cast<float>(maxCol) / 2.0f;
            entity->position.z = (static_cast<float>(row - 1) - entity->position.z) - static_cast<float>(row) / 2.0f;
        }

        meshes.emplace("Plane", std::make_shared<applesauce::Mesh>(makePlaneMesh(maxCol - 1, row - 1, checkerMaterial)));
        spawn(new Floor());

        glGenFramebuffers(1, &depthMapFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        depthMap = std::make_shared<applesauce::DepthTexture2D>(SHADOW_WIDTH, SHADOW_HEIGHT);
        depthMap->bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap->glId(), 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        applesauce::Input::init();
    }

    void update(float dt) override
    {
        entities.erase(std::remove_if(entities.begin(), entities.end(), [](const auto &e)
                                      { return e->isPendingDestruction; }),
                       entities.end());
        for (auto &entity : entities)
        {
            entity->update(dt);

            if (entity->collidable)
            {
                glm::vec2 ejectionVector;
                // TODO: Can we avoid updating this matrix twice?
                // This is collision vs walls specifically
                entity->modelMatrix = glm::translate(glm::mat4{1.0f}, entity->position) * glm::mat4(entity->orientation);
                if (tm.checkCollision(quadFromEntity(*entity, entity->collisionSize), ejectionVector))
                {
                    entity->position.x += ejectionVector.x;
                    entity->position.z += ejectionVector.y;
                    entity->onTouch();
                }
            }

            // Update modelMatrix of all entities in preparation for render
            entity->modelMatrix = glm::translate(glm::mat4{1.0f}, entity->position) * glm::mat4(entity->orientation);
        }

        for (auto i = entities.begin(); i != entities.end(); i++)
        {
            for (auto j = i; ++j != entities.end();)
            {
                auto &entA = *i;
                auto &entB = *j;

                // If either entity is the originator of the other, skip
                if ((entA->originator != nullptr && entA->originator == entB.get()) || (entB->originator != nullptr && entB->originator == entA.get()))
                    continue;

                // TODO: Different collision types
                // The shell only needs to overlap on the AABB. Nothing more.
                // Tank to tank is similar to tank to wall. The tanks should
                // stop each other from penetrating.
                if (checkCollision(aabbFromEntity(*entA), aabbFromEntity(*entB)))
                {
                    // Figure out a better way of telling the tanks are bumping (maybe even just have them as a permenant pair)
                    if (std::dynamic_pointer_cast<Tenk>(entA) && std::dynamic_pointer_cast<Tenk>(entB))
                    {
                        Quad quadA = quadFromEntity(*entA, entA->collisionSize);
                        Quad quadB = quadFromEntity(*entB, entB->collisionSize);

                        float minOverlap = 0;
                        glm::vec2 normal;
                        if (checkCollision(quadA, quadB, normal, minOverlap))
                        {
                            glm::vec3 normal3{normal.x, 0, normal.y};
                            if (glm::dot(normal3, glm::normalize(entA->position - entB->position)) > 0)
                            {
                                normal3 = -normal3;
                            }
                            entA->position -= normal3 * minOverlap * 0.5f;
                            entB->position += normal3 * minOverlap * 0.5f;
                        }
                    }
                    entA->onTouch(*entB);
                    entB->onTouch(*entA);
                }
            }
        }

        glm::vec3 tenkCenter = tenks[0]->position + (tenks[1]->position - tenks[0]->position) * 0.5f;
        cameraTarget += (tenkCenter - cameraTarget) * dt;

        float tenksDist = glm::distance(tenks[0]->position, tenks[1]->position);
        float targetDist = (tenksDist - dist) + 4.0f;
        dist += targetDist * dt;
    }

    void display() override
    {
        glm::vec3 lightDir = glm::normalize(glm::vec3{0.5, 1, 0.25});

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glCullFace(GL_FRONT);

        static float lightDist = 10.0f;
        static float lightSize = 16.0f;
        static float lightNear = 0.1f;
        static float lightFar = 20.0f;
        shadow->use();
        glm::mat4 lightSpaceMatrix;
        { // Shadow map part
            glm::mat4 view = glm::lookAt(lightDir * lightDist,
                                         glm::vec3(0),
                                         glm::vec3(0, 1, 0));
            glm::mat4 projection = glm::ortho(-lightSize, lightSize, -lightSize, lightSize, lightNear, lightFar);
            lightSpaceMatrix = projection * view;

            for (const auto &entity : entities)
            {
                glm::mat4 MVPMatrix = lightSpaceMatrix * entity->modelMatrix;

                shadow->set("MVPMatrix", MVPMatrix);

                for (const auto &primitive : entity->mesh->primitives)
                {
                    primitive.vertexArray->bind();
                    primitive.indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                    glDrawElements(GL_TRIANGLES, primitive.elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
                }
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glCullFace(GL_BACK);
        glEnable(GL_FRAMEBUFFER_SRGB);

        shader->use();

        glActiveTexture(GL_TEXTURE0 + 1);
        depthMap->bind();

        const auto [width, height] = window.framebufferSize();
        camera.viewport = {width, height};
        glViewport(0, 0, width, height);

        window.clear({0.01f, 0.01f, 0.01f, 1.0f});

        camera.position = glm::mat3(glm::yawPitchRoll(theta, pitch, 0.0f)) * glm::vec3{0, 0, -dist};
        glm::mat4 view = camera.lookAtMatrix(cameraTarget);
        glm::mat4 projection = camera.projectionMatrix();

        glm::vec3 LightDirection = glm::mat3(view) * lightDir;

        // This should have a translate of 0.5 in each coordinate, but instead scale is influencing,
        // so some ordering is wrong. The 1.0f translate is a hacky fix.
        glm::mat4 shadowMatrix = glm::translate(glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f}), glm::vec3{1.0f});

        shadowMatrix *= lightSpaceMatrix;

        for (const auto &entity : entities)
        {
            glm::mat4 modelView = view * entity->modelMatrix;
            glm::mat3 normalMatrix = glm::mat3(modelView);

            glm::mat4 MVPMatrix = projection * modelView;
            glm::mat4 LightViewMatrix = shadowMatrix * entity->modelMatrix;

            shader->set("MVPMatrix", MVPMatrix);
            shader->set("ModelViewMatrix", modelView);
            shader->set("LightViewMatrix", LightViewMatrix);
            shader->set("NormalMatrix", normalMatrix);
            shader->set("AmbientSky", triAmbient.sky);
            shader->set("AmbientEquator", triAmbient.equator);
            shader->set("AmbientGround", triAmbient.ground);
            shader->set("LightColor", glm::vec3{1.0, 1.0, 1.0});
            shader->set("LightDirection", LightDirection);

            shader->set("albedo", 0);
            shader->set("shadowMap", 1);

            for (const auto &primitive : entity->mesh->primitives)
            {
                if (primitive.material)
                {
                    const auto &material = primitive.material;
                    shader->set("Color", primitive.material->baseColor);
                    shader->set("MetallicFactor", primitive.material->metallicFactor);
                    shader->set("RoughnessFactor", primitive.material->roughnessFactor);

                    glActiveTexture(GL_TEXTURE0);
                    if (material->baseTexture)
                        material->baseTexture->bind();
                    else
                        glBindTexture(GL_TEXTURE_2D, 0);
                }
                else
                {
                    shader->set("Color", glm::vec3(1.0, 1.0, 1.0));
                    shader->set("MetallicFactor", 0.0f);
                    shader->set("RoughnessFactor", 0.25f);
                }
                primitive.vertexArray->bind();
                primitive.indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                glDrawElements(GL_TRIANGLES, primitive.elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Adjustment");

        ImGui::ColorEdit3("sky", &triAmbient.sky[0]);
        ImGui::ColorEdit3("equator", &triAmbient.equator[0]);
        ImGui::ColorEdit3("ground", &triAmbient.ground[0]);

        ImGui::ColorEdit3("tenk", (float *)&getMesh("Tenk")->primitives.front().material->baseColor);

        ImGui::ColorEdit3("floorColor", (float *)&getMesh("Plane")->primitives.front().material->baseColor);
        ImGui::SliderFloat("floorRoughness", (float *)&getMesh("Plane")->primitives.front().material->roughnessFactor, 0, 1.0f);
        ImGui::SliderFloat("floorMetallic", (float *)&getMesh("Plane")->primitives.front().material->metallicFactor, 0, 1.0f);

        ImGui::SliderFloat("wallRoughness", (float *)&getMesh("Wall")->primitives.front().material->roughnessFactor, 0, 1.0f);
        ImGui::SliderFloat("wallMetallic", (float *)&getMesh("Wall")->primitives.front().material->metallicFactor, 0, 1.0f);

        ImGui::SliderFloat("lightDist", &lightDist, 0.001f, 40.0f);
        ImGui::SliderFloat("lightSize", &lightSize, 0.001f, 40.0f);
        ImGui::SliderFloat("lightNear", &lightNear, 0.001f, 40.0f);
        ImGui::SliderFloat("lightFar", &lightFar, 0.001f, 40.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /*
                glDisable(GL_DEPTH_TEST);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, depthMap);

                quad->use();

                renderQuad();
                */
    }

    void cleanUp() override
    {
        std::cout << "Camera Stats:\n";
        std::cout << "\tPitch: " << pitch << std::endl;
        std::cout << "\tTheta: " << theta << std::endl;
        std::cout << "\tDist: " << dist << std::endl;
    }

    std::shared_ptr<applesauce::Entity> spawn(applesauce::Entity *entity, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{glm::vec3{0}}) override
    {
        auto e = std::shared_ptr<applesauce::Entity>(entity);
        e->init(*this);
        e->position = position;
        e->orientation = orientation;
        e->world = this;
        entities.emplace_back(e);
        return entities.back();
    }

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> shadow;
    std::shared_ptr<Shader> quad;

    std::list<std::shared_ptr<applesauce::Entity>> entities;

    std::unordered_map<std::string, std::shared_ptr<applesauce::Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<applesauce::Texture>> textures;

    std::vector<std::shared_ptr<Tenk>> tenks;

    Camera camera;
    glm::vec3 cameraTarget = glm::vec3{0};

    float pitch = 0.912121;
    float theta = 3.2649;
    float dist = 21.7568;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;

    glm::vec3 ambient{0.3, 0.3, 0.3};

    struct Ambient
    {
        glm::vec3 sky;
        glm::vec3 equator;
        glm::vec3 ground;
    };
    Ambient triAmbient{
        {0.779f, 0.390f, 0.000f},
        {0.377f, 0.133f, 0.392f},
        {0.102f, 0.002f, 0.127f},
    };

    // Shadow map bits
    GLuint depthMapFBO;
    std::shared_ptr<applesauce::DepthTexture2D> depthMap;

    TileMap tm;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}