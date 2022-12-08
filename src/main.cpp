#include "applesauce/App.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
#include "applesauce/Camera.h"

#include "util/gltf.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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
#include <filesystem>
#include <fstream>
#include <list>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

unsigned int quadVAO = 0;
unsigned int quadVBO;
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

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

static constexpr glm::vec3 FLOOR_COLOR = glm::vec3{1.0, 0.6, 0.1};
static constexpr glm::vec3 WALL_COLOR = glm::vec3{0.6, 0.6, 1.0};

static float MAX_DIST = 30.0f;
static constexpr float MIN_DIST = 8.0f;
static constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

static size_t nextTankColor = 0;
static glm::vec3 TANK_COLORS[] = {
    glm::vec3{58.0f / 255.0, 58.0f / 255.0, 156.0f / 255.0}, // blue
    glm::vec3{163.0f / 255.0, 58.0f / 255.0, 36.0f / 255.0}, // red
    glm::vec3{1, 1, 1},                                      // white
    glm::vec3{1, 1, 0},                                      // yellow
    glm::vec3{0, 0, 0},                                      // black
};

std::string readFileText(const char *filename)
{
    std::ifstream f{filename};
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

std::shared_ptr<Shader> loadShader(const char *name)
{
    static const std::string vertexShaderExt = ".vs.glsl";
    static const std::string fragmentShaderExt = ".fs.glsl";

    std::filesystem::path assetsPath = "assets/shaders";
    std::filesystem::path vertexShaderPath = assetsPath / (std::string(name) + vertexShaderExt);
    std::filesystem::path fragmentShaderPath = assetsPath / (std::string(name) + fragmentShaderExt);

    std::cout << "Loading shader " << name << " from:\n";
    std::cout << "\t" << vertexShaderPath << "\n";
    std::cout << "\t" << fragmentShaderPath << std::endl;

    const auto vertex_shader_text = readFileText(vertexShaderPath.c_str());
    const auto fragment_shader_text = readFileText(fragmentShaderPath.c_str());

    auto shader = std::make_shared<Shader>();
    shader->add_vertex_stage(vertex_shader_text);
    shader->add_fragment_stage(fragment_shader_text);

    if (!shader->compile_and_link())
    {
        std::cerr << shader->error_log() << std::endl;
        return nullptr;
    }

    return shader;
}

std::ostream &operator<<(std::ostream &os, const Window::MouseHandler::Button &b)
{
    switch (b)
    {
    case Window::MouseHandler::Button::left:
        os << "left";
        break;
    case Window::MouseHandler::Button::right:
        os << "right";
        break;
    case Window::MouseHandler::Button::middle:
        os << "middle";
        break;
    default:
        os << "unknown mouse button";
        break;
    }
    return os;
}

// Experimental Mesh structure
struct SubMesh
{
    std::shared_ptr<applesauce::VertexArray> array;
    std::vector<std::shared_ptr<applesauce::Buffer>> buffers;
    std::shared_ptr<applesauce::Buffer> indexBuffer;
    int indexBufferByteOffset;
    int elementCount;
};

struct Mesh
{
    std::list<SubMesh> submeshes;
};

static applesauce::VertexAttribute vertexAttribFromName(const std::string &name)
{
    if (name == "POSITION")
        return applesauce::VertexAttribute::position;
    else if (name == "NORMAL")
        return applesauce::VertexAttribute::normal;
    else if (name == "TEXCOORD_0")
        return applesauce::VertexAttribute::texcoord;
    else
        return applesauce::VertexAttribute::none;
}

std::unordered_map<std::string, Mesh> loadMeshes(const char *filename)
{
    std::unordered_map<std::string, Mesh> result;

    std::string gltfText(readFileText(filename));
    const auto gltf = glTFFromString(gltfText.c_str());

    std::vector<std::shared_ptr<applesauce::Buffer>> buffers;
    // Load up buffers, these are going directly into OpenGL, which is arguable
    for (const auto &gltfBuffer : gltf.buffers)
    {
        buffers.emplace_back(std::make_shared<applesauce::Buffer>(&gltfBuffer.getBytes()[0], gltfBuffer.byteLength));
    }

    for (const auto &gltfMesh : gltf.meshes)
    {
        std::list<SubMesh> submeshes;
        for (const auto &gltfMeshPrimitive : gltfMesh.primitives)
        {
            auto vertexArray = std::make_shared<applesauce::VertexArray>();
            vertexArray->bind();
            for (const auto &[accessorName, accessorIndex] : gltfMeshPrimitive.attributes)
            {
                const auto &accessor = gltf.accessors[accessorIndex];
                const auto &bufferView = gltf.bufferViews[accessor.bufferView];
                const auto offset = bufferView.byteOffset + accessor.byteOffset;
                const auto vAttrib = vertexAttribFromName(accessorName);

                applesauce::VertexBufferDescription desc{
                    {
                        vAttrib,
                        accessor.componentCount(),
                        offset,
                        bufferView.byteStride,
                    },
                };
                vertexArray->addVertexBuffer(*buffers[bufferView.buffer], desc);
            }

            const auto &indicesAccessor = gltf.accessors[gltfMeshPrimitive.indices];
            const auto &indicesBufferView = gltf.bufferViews[indicesAccessor.bufferView];

            submeshes.emplace_back(SubMesh{
                vertexArray,
                buffers,
                buffers[indicesBufferView.buffer],
                indicesBufferView.byteOffset + indicesAccessor.byteOffset,
                indicesAccessor.count,
            });
            std::cout << "Index buffer offset:" << indicesBufferView.byteOffset + indicesAccessor.byteOffset << std::endl;
            vertexArray->unbind();
        }
        result.emplace(gltfMesh.name, Mesh{submeshes});
    }

    return result;
}

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

    class Tank
    {
        enum class Steer
        {
            neutral,
            left,
            right
        };

    public:
        Tank(Entity *entity) : entity(entity) {}

        void steerLeft()
        {
            steering = Steer::left;
        }
        void steerRight()
        {
            steering = Steer::right;
        }
        void releaseSteering()
        {
            steering = Steer::neutral;
        }

        const glm::vec3 position() const
        {
            return entity->position;
        }

        void advance()
        {
            advancing = true;
        }

        void halt()
        {
            advancing = false;
        }

        void update()
        {
            static float steerRate = 0.03;
            static float speed = 0.06;
            switch (steering)
            {
            case Steer::left:
                entity->angle += steerRate;
                break;
            case Steer::right:
                entity->angle -= steerRate;
                break;
            case Steer::neutral:
                break;
            }

            if (advancing)
            {
                glm::vec3 direction = glm::rotateY(glm::vec3{0.0, 0.0, -1.0}, entity->angle);
                entity->position += direction * speed;
            }
        }

        Entity *entity;

    private:
        Steer steering = Steer::neutral;
        bool advancing = false;
    };

public:
    void onKeyDown(int key) override
    {
        std::cout << "KEY DOWN: " << key << std::endl;
        if (players.size() < 2 || players[0] == nullptr || players[1] == nullptr)
            return;
        if (key == GLFW_KEY_A)
        {
            players[0]->steerLeft();
        }
        else if (key == GLFW_KEY_D)
        {
            players[0]->steerRight();
        }
        else if (key == GLFW_KEY_W)
        {
            players[0]->advance();
        }

        if (key == GLFW_KEY_LEFT)
        {
            players[1]->steerLeft();
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            players[1]->steerRight();
        }
        else if (key == GLFW_KEY_UP)
        {
            players[1]->advance();
        }
    }

    void onKeyUp(int key) override
    {
        if (players.size() < 2 || players[0] == nullptr || players[1] == nullptr)
            return;

        if (key == GLFW_KEY_A || key == GLFW_KEY_D)
        {
            players[0]->releaseSteering();
        }
        else if (key == GLFW_KEY_W)
        {
            players[0]->halt();
        }
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
        {
            players[1]->releaseSteering();
        }
        else if (key == GLFW_KEY_UP)
        {
            players[1]->halt();
        }
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;

        ImGui::StyleColorsDark();

        const char *glsl_version = "#version 150";

        ImGui_ImplGlfw_InitForOpenGL(window.glfwWindow(), true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        srand(std::time(0));

        window.setScrollHandler(this);
        window.setMouseHandler(this);
        window.setKeyHandler(this);

        // Loads from assets/shaders/basic.fs.glsl and assets/shaders/basic.vs.glsl
        shader = loadShader("basic");
        shadow = loadShader("shadow");
        quadShader = loadShader("quad");

        const auto levelMeshes = loadMeshes("assets/gltf/wall-and-floor.gltf");
        const auto tenkMeshes = loadMeshes("assets/gltf/tenk7.gltf");

        meshGroups.resize(3);
        meshGroups[0].push_back(levelMeshes.at("Floor"));
        meshGroups[1].push_back(levelMeshes.at("Wall"));
        meshGroups[2].push_back(tenkMeshes.at("Tenk"));

        glfwSwapInterval(1);

        const auto [width, height] = window.framebufferSize();
        glViewport(0, 0, width, height);

        const char *playField = "********************************\n"
                                "*                              *\n"
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
                                "*                              *\n"
                                "*                              *\n"
                                "********************************";

        std::stringstream stream(playField);
        std::string line;

        int row = 0;
        int maxCol = -1;
        while (std::getline(stream, line, '\n'))
        {
            int col = 0;
            for (const auto &character : line)
            {
                size_t idx = character == '*' ? 1 : 0;
                glm::vec3 color = character == '*' ? WALL_COLOR : FLOOR_COLOR;
                entities.push_back({{col, 0, row}, 0, idx, color});

                // If there's a T also put a tank, cause why not
                if (character == 'T')
                {
                    entities.push_back({{col, 0, row}, 0, 2, TANK_COLORS[nextTankColor++ % 5]});
                }

                col++;
            }
            maxCol = std::max(maxCol, col);
            row++;
        }

        for (auto &entity : entities)
        {
            entity.position.x -= maxCol / 2;
            entity.position.z -= row / 2;

            if (entity.meshIndex == 2)
            {
                players.emplace_back(std::make_shared<Tank>(&entity));
            }
        }
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
        for (auto &player : players)
        {
            player->update();
        }

        glm::vec3 midPoint = (players[0]->position() + players[1]->position()) / 2.0f;
        float tankDist = glm::distance(players[0]->position(), players[1]->position());

        glm::vec3 movementDirection = midPoint - cameraTarget;
        cameraVelocity += movementDirection * .001f;

        cameraVelocity += -cameraVelocity * .1f;
        cameraTarget += cameraVelocity;

        float distTarget = MIN_DIST + (MAX_DIST - MIN_DIST) * (tankDist / 25.0f);
        distVelocity += (distTarget - dist) * .005;

        distVelocity += -distVelocity * .1f;
        dist += distVelocity;
    }

    void display() override
    {
        glm::vec3 lightDir = glm::mat3(glm::yawPitchRoll(lightTheta, lightPitch, 0.0f)) * glm::vec3{0, 0, -1.0};

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
        glm::mat4 view = camera.lookAtMatrix(cameraTarget);
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
        quadShader->use();

        glDisable(GL_DEPTH_TEST);
        // renderQuad();

        // GUI tuff
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Adjustment");

        ImGui::ColorEdit3("ambient", (float *)&ambient[0]);
        ImGui::ColorEdit3("tank 1 color", (float *)&players[0]->entity->color);
        ImGui::ColorEdit3("tank 2 color", (float *)&players[1]->entity->color);

        ImGui::SliderFloat("lightPitch", &lightPitch, 0, M_PI);
        ImGui::SliderFloat("lightTheta", &lightTheta, 0, M_PI * 2);
        ImGui::SliderFloat("lightDist", &lightDist, 1, 50);
        ImGui::SliderFloat("lightSize", &lightSize, 1, 50);

        ImGui::SliderFloat("lightPitch", &lightPitch, 0, M_PI);
        ImGui::SliderFloat("lightTheta", &lightTheta, 0, M_PI * 2);
        ImGui::SliderFloat("lightDist", &lightDist, 1, 50);
        ImGui::SliderFloat("lightSize", &lightSize, 1, 50);

        ImGui::SliderFloat("SpecularPower", &specularPower, 0.0f, 256.0f);
        ImGui::SliderFloat("SpecularStrength", &specularStrength, 0.0f, 1.0f);

        ImGui::SliderFloat("camera.FOV", &camera.fieldOfVision, 0.1f, 120.0f);
        ImGui::SliderFloat("MAX DIST.FOV", &MAX_DIST, 0.1f, 120.0f);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    std::shared_ptr<Shader> quadShader;

    std::vector<Entity> entities;

    std::vector<std::vector<Mesh>> meshGroups;

    std::vector<std::shared_ptr<Tank>> players;

    float pitch = 0.955591;
    float theta = 2.90973;
    float dist = 25.1983;
    float distVelocity = 0;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;

    Camera camera;
    glm::vec3 cameraTarget{0};
    glm::vec3 cameraVelocity{0};

    glm::vec3 ambient{87.0f / 255.0f, 57.0 / 255.0f, 129.0f / 255.0f};

    float lightPitch = 2.119;
    float lightTheta = 0.839;
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