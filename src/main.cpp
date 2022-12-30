#define _USE_MATH_DEFINES

#include "applesauce/App.h"
#include "applesauce/Input.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
#include "applesauce/Texture.h"
#include "applesauce/Camera.h"
#include "applesauce/Mesh.h"

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
#include <list>
#include <cmath>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

static unsigned int quadVAO;
static unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            -0.5f,
            0.5f,
            0.0f,
            0.0f,
            1.0f,
            -0.5f,
            -0.5f,
            0.0f,
            0.0f,
            0.0f,
            0.5f,
            0.5f,
            0.0f,
            1.0f,
            1.0f,
            0.5f,
            -0.5f,
            0.0f,
            1.0f,
            0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

static constexpr unsigned int SHADOW_WIDTH = 2048,
                              SHADOW_HEIGHT = 2048;

class ResourceManager
{
public:
    virtual std::shared_ptr<applesauce::Mesh> getMesh(const std::string &) = 0;
    virtual std::shared_ptr<applesauce::Texture2D> getTexture(const std::string &) = 0;
};

struct IWorld;
struct Entity
{
    glm::vec3 position = glm::vec3{0};
    glm::quat orientation = glm::quat{};
    std::shared_ptr<applesauce::Mesh> mesh = nullptr;
    std::shared_ptr<applesauce::Texture> texture = nullptr;

    glm::mat4 modelMatrix = glm::mat4{1.0f};

    IWorld *world = nullptr;

    virtual void init(ResourceManager &) {}
    virtual void update(float) {}
    virtual ~Entity() {}
    void destroy()
    {
        isPendingDestruction = true;
    }

    bool isPendingDestruction = false;
};

struct IWorld
{
    virtual std::shared_ptr<Entity> spawn(Entity *, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{glm::vec3{0}}) = 0;
};

float fRand(float max)
{
    return max * static_cast<float>(rand() % 10000) / 10000.0f;
}

class Shell : public Entity
{
    int timer;

    void init(ResourceManager &rm)
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

class TinyBlock : public Entity
{
    glm::vec3 velocity;
    float timer;

    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("TinyBox");
        texture = rm.getTexture("White Square");
        timer = rand() % 400;

        float speed = fRand(12.0f) + 0.6f;
        glm::vec3 vel{0, 1.0f, 0};
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

class Block : public Entity
{
    static constexpr float topSpinSpeed = M_PI * 4;

    float timeLimit;
    float timer = 0;
    void init(ResourceManager &rm)
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
        orientation = glm::rotate(orientation, spinSpeed, glm::vec3{0, 1.0f, 0});
        timer += dt;
    }
};

class Floor : public Entity
{
    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("Plane");
        texture = rm.getTexture("Checker");
    }
    void update(float)
    {
    }
};

class Tenk : public Entity
{
    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("Tenk");
        texture = rm.getTexture("White");
        std::cout << "Spawned Tank" << std::endl;
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
            glm::vec3 direction = glm::mat3(orientation) * glm::vec3{0, 0, -1.0f};
            position += direction * speed * dt;
        }
        if (applesauce::Input::wasJustPressed(GLFW_KEY_SPACE))
        {
            auto shell = std::dynamic_pointer_cast<Shell>(world->spawn(new Shell, position + glm::vec3{0, 0.75, 0}));
            if (shell)
            {
                std::cout << "BOOM!" << std::endl;
                glm::vec3 direction = glm::mat3(orientation) * glm::vec3{0, 0, -1.0f};
                shell->velocity = direction * 20.0f;
            }
        }

        orientation = glm::rotate(orientation, spinSpeed * dt, glm::vec3{0, 1.0f, 0});
    }
};

class Triangles : public App,
                  public IWorld,
                  public ResourceManager,
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

        meshes.emplace("TinyBox", std::make_shared<applesauce::Mesh>(makeBoxMesh(0.25f)));
        meshes.emplace("Box", std::make_shared<applesauce::Mesh>(makeBoxMesh(1.0f)));
        meshes.emplace("Plane", std::make_shared<applesauce::Mesh>(makePlaneMesh(20)));

        for (const auto &[name, mesh] : applesauce::loadMeshes("assets/gltf/tenk7.gltf"))
        {
            meshes.emplace(name, std::make_shared<applesauce::Mesh>(mesh));
        }

        meshes.emplace("Plane", std::make_shared<applesauce::Mesh>(makePlaneMesh(20)));

        spawn(new Floor());
        spawn(new Tenk());

        int numberOfObjects = 20;
        float radius = 5.0f;

        for (int i = 0; i < numberOfObjects; i++)
        {
            float angle = i * M_PI * 2 / numberOfObjects;
            float x = cosf(angle) * radius;
            float z = sinf(angle) * radius;
            glm::vec3 position{x, 0.5, z};

            glm::quat orientation{glm::vec3{0, -angle, 0}};
            spawn(new Block(), position, orientation);
        }

        // Init shadow mapping bits

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
            // Update modelMatrix of all entities in preparation for render
            entity->modelMatrix = glm::translate(glm::mat4{1.0f}, entity->position) * glm::mat4(entity->orientation);
        }
    }

    void display() override
    {
        glm::vec3 lightDir = glm::normalize(glm::vec3{0.5, 1, 0.25});

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        /*
                glEnable(GL_POLYGON_OFFSET_FILL);
                glPolygonOffset(2.0f, 4.0f);
                */

        glCullFace(GL_FRONT);

        static float lightDist = 11.2;
        static float lightSize = 12.0f;
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
        glm::mat4 view = camera.lookAtMatrix(glm::vec3{0});
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
            // shader->set("Ambient", ambient);
            shader->set("AmbientSky", triAmbient.sky);
            shader->set("AmbientEquator", triAmbient.equator);
            shader->set("AmbientGround", triAmbient.ground);
            shader->set("LightColor", glm::vec3{1.0, 1.0, 1.0});
            shader->set("LightDirection", LightDirection);

            shader->set("albedo", 0);
            shader->set("shadowMap", 1);

            glActiveTexture(GL_TEXTURE0);
            if (entity->texture)
                entity->texture->bind();
            else
                glBindTexture(GL_TEXTURE_2D, 0);

            for (const auto &primitive : entity->mesh->primitives)
            {
                if (primitive.material)
                {
                    shader->set("Color", primitive.material->baseColor);
                    shader->set("MetallicFactor", primitive.material->metallicFactor);
                    shader->set("RoughnessFactor", primitive.material->roughnessFactor);
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

    std::shared_ptr<Entity> spawn(Entity *entity, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{glm::vec3{0}}) override
    {
        auto e = std::shared_ptr<Entity>(entity);
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

    std::list<std::shared_ptr<Entity>> entities;

    std::unordered_map<std::string, std::shared_ptr<applesauce::Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<applesauce::Texture>> textures;

    Camera camera;
    float pitch = 0.363528;
    float theta = 3.3335;
    float dist = 10.1983;

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
        {0.212, 0.227, 0.259},
        {0.114, 0.125, 0.133},
        {0.047, 0.043, 0.035},
    };

    // Shadow map bits
    GLuint depthMapFBO;
    std::shared_ptr<applesauce::DepthTexture2D> depthMap;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}