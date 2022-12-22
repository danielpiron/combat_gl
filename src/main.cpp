#define _USE_MATH_DEFINES

#include "applesauce/App.h"
#include "applesauce/AltMesh.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
#include "applesauce/Texture.h"
#include "applesauce/Camera.h"
// #include "applesauce/Mesh.h"

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

#include <png.h>

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

static constexpr unsigned int SHADOW_WIDTH = 1024,
                              SHADOW_HEIGHT = 1024;

std::shared_ptr<applesauce::Texture> try_png(const char *filename)
{
    png_image image;
    /* Only the image structure version number needs to be set. */
    std::memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_file(&image, filename))
    {
        image.format = PNG_FORMAT_RGBA;
        auto buffer = reinterpret_cast<png_bytep>(malloc(PNG_IMAGE_SIZE(image)));

        if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/,
                                  NULL /*colormap for PNG_FORMAT_FLAG_COLORMAP */))
        {
            auto tex = std::make_shared<applesauce::Texture>();
            tex->setMinFilter(applesauce::Texture::Filter::linearMipMapLinear);
            tex->setMagFilter(applesauce::Texture::Filter::linear);

            auto ptr = reinterpret_cast<GLubyte *>(buffer);
            tex->setImage(image.width, image.height, applesauce::Texture::Format::rgba, ptr);
            tex->generateMipmaps();

            return tex;
        }
    }
    return nullptr;
}

class ResourceManager
{
public:
    virtual std::shared_ptr<Mesh> getMesh(const std::string &) = 0;
    virtual std::shared_ptr<applesauce::Texture2D> getTexture(const std::string &) = 0;
};

struct IWorld;
struct Entity
{
    glm::vec3 position = glm::vec3{0};
    glm::quat orientation = glm::quat{};
    std::shared_ptr<Mesh> mesh = nullptr;
    std::shared_ptr<applesauce::Texture> texture = nullptr;

    glm::mat4 modelMatrix = glm::mat4{1.0f};

    IWorld *world = nullptr;

    virtual void init(ResourceManager &) {}
    virtual void update() {}
    virtual ~Entity() {}
    void destroy()
    {
        isPendingDestruction = true;
    }

    bool isPendingDestruction = false;
};

struct IWorld
{
    virtual void spawn(Entity *, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{}) = 0;
};

float fRand(float max)
{
    return max * static_cast<float>(rand() % 10000) / 10000.0f;
}

class TinyBlock : public Entity
{
    glm::vec3 velocity;
    float timer;

    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("TinyBox");
        texture = rm.getTexture("White Square");
        timer = rand() % 400;

        float speed = fRand(0.2) + 0.01;
        glm::vec3 vel{0, 1.0f, 0};
        vel = glm::mat3(glm::yawPitchRoll(fRand(1.0f) - 0.5f, 0.0f, fRand(1.0f) - 0.5f)) * vel;
        velocity = vel * speed;
    }
    void update()
    {
        if (timer == 0)
        {
            destroy();
        }
        velocity += glm::vec3(0, -0.001f, 0);
        position += velocity;

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
    static constexpr float topSpinSpeed = 0.3f;

    int endTime;
    int timer = 0;
    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("Box");
        texture = rm.getTexture("White Square");
        endTime = rand() % 2000;
    }
    void update()
    {
        if (timer >= endTime)
        {
            size_t spawnCount = static_cast<size_t>(rand() % 10) + 10;
            for (size_t i = 0; i <= spawnCount; ++i)
            {
                world->spawn(new TinyBlock, position, glm::quat{});
            }
            destroy();
        }
        const float spinSpeed = (static_cast<float>(timer) / static_cast<float>(endTime)) * topSpinSpeed;
        orientation = glm::rotate(orientation, spinSpeed, glm::vec3{0, 1.0f, 0});
        timer++;
    }
};

class Floor : public Entity
{
    void init(ResourceManager &rm)
    {
        mesh = rm.getMesh("Plane");
        texture = rm.getTexture("Checker");
    }
    void update()
    {
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

    std::shared_ptr<Mesh> getMesh(const std::string &key) override
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

        textures.emplace("Checker", try_png("assets/textures/Checker.png"));
        textures.emplace("White Square", try_png("assets/textures/White Square.png"));

        meshes.emplace("TinyBox", std::make_shared<Mesh>(makeBoxMesh(0.25f)));
        meshes.emplace("Box", std::make_shared<Mesh>(makeBoxMesh(1.0f)));
        meshes.emplace("Plane", std::make_shared<Mesh>(makePlaneMesh(20)));

        spawn(new Floor());

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
    }

    void update() override
    {
        entities.erase(std::remove_if(entities.begin(), entities.end(), [](const auto &e)
                                      { return e->isPendingDestruction; }),
                       entities.end());
        for (auto &entity : entities)
        {
            entity->update();
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

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

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

                auto &mesh = entity->mesh;
                mesh->vertexArray->bind();
                mesh->indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                glDrawElements(GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        shader->use();

        glActiveTexture(GL_TEXTURE0 + 1);
        depthMap->bind();

        const auto [width, height] = window.framebufferSize();
        camera.viewport = {width, height};
        glViewport(0, 0, width, height);

        window.clear({0.1f, 0.1f, 0.1f, 1.0f});

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
            shader->set("Color", glm::vec3(0.5, 0.5, 0.5));
            // shader->set("Ambient", ambient);
            shader->set("AmbientSky", triAmbient.sky);
            shader->set("AmbientEquator", triAmbient.equator);
            shader->set("AmbientGround", triAmbient.ground);
            shader->set("LightColor", glm::vec3{1.0, 1.0, 1.0});
            shader->set("LightDirection", LightDirection);
            shader->set("SpecularPower", specularPower);
            shader->set("SpecularStrength", specularStrength);

            shader->set("albedo", 0);
            shader->set("shadowMap", 1);

            glActiveTexture(GL_TEXTURE0);
            entity->texture->bind();

            auto &mesh = entity->mesh;
            mesh->vertexArray->bind();
            mesh->indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
            glDrawElements(GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Adjustment");

        ImGui::ColorEdit3("sky", &triAmbient.sky[0]);
        ImGui::ColorEdit3("equator", &triAmbient.equator[0]);
        ImGui::ColorEdit3("ground", &triAmbient.ground[0]);

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

    void spawn(Entity *entity, const glm::vec3 &position = glm::vec3{0}, const glm::quat &orientation = glm::quat{}) override
    {
        auto e = std::shared_ptr<Entity>(entity);
        e->init(*this);
        e->position = position;
        e->orientation = orientation;
        e->world = this;
        entities.emplace_back(e);
    }

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Shader> shadow;
    std::shared_ptr<Shader> quad;

    std::list<std::shared_ptr<Entity>> entities;

    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
    std::unordered_map<std::string, std::shared_ptr<applesauce::Texture>> textures;

    Camera camera;
    float pitch = 0.363528;
    float theta = 3.3335;
    float dist = 10.1983;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;

    glm::vec3 ambient{0.3, 0.3, 0.3};

    float specularPower = 32.0f;
    float specularStrength = 1.0f;

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