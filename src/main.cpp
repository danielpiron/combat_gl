#include "applesauce/App.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"

#define GLM_SWIZZLE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <algorithm>
#include <memory>
#include <ostream>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

#define VERTEX_PROPERTY(I, T, M) glVertexAttribPointer(I,                                                 \
                                                       sizeof(T::M) / sizeof(decltype(T::M)::value_type), \
                                                       GL_FLOAT,                                          \
                                                       GL_FALSE,                                          \
                                                       sizeof(T),                                         \
                                                       reinterpret_cast<void *>(offsetof(T, M)))

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

class Triangles : public App, public Window::ScrollHandler, public Window::MouseHandler
{
public:
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

        const char *vertex_shader_text = R"(
           #version 330 core
           layout (location = 0) in vec3 vPosition;
           layout (location = 1) in vec3 vNormal;

           uniform mat4 mMVP;

           out vec4 color;
           void main() {
            gl_Position = mMVP * vec4(vPosition, 1);
            color = vec4(vNormal * 0.5 + vec3(0.5), 1);
           })";

        const char *fragment_shader_text = R"(
        #version 330 core
        in vec4 color;
        out vec4 fColor;
        void main() {
            fColor = color;
        }
    )";

        shader = std::make_shared<Shader>();

        shader->add_vertex_stage(vertex_shader_text);
        shader->add_fragment_stage(fragment_shader_text);

        if (!shader->compile_and_link())
        {
            std::cerr << shader->error_log() << std::endl;
            return;
        }

        glUseProgram(shader->glId());

        cubeBuffer = std::make_shared<applesauce::VertexBuffer<Vertex>>(std::initializer_list<Vertex>{
            {{0.5, 1.0, -0.5}, {0.0, 1.0, 0.0}},
            {{-0.5, 1.0, 0.5}, {0.0, 1.0, 0.0}},
            {{0.5, 1.0, 0.5}, {0.0, 1.0, 0.0}},
            {{0.5, 0.0, 0.5}, {0.0, 0.0, 1.0}},
            {{-0.5, 1.0, 0.5}, {0.0, 0.0, 1.0}},
            {{-0.5, 0.0, 0.5}, {0.0, 0.0, 1.0}},
            {{-0.5, 0.0, 0.5}, {-1.0, 0.0, 0.0}},
            {{-0.5, 1.0, -0.5}, {-1.0, 0.0, 0.0}},
            {{-0.5, 0.0, -0.5}, {-1.0, 0.0, 0.0}},
            {{-0.5, 0.0, -0.5}, {0.0, -1.0, -0.0}},
            {{0.5, 0.0, 0.5}, {0.0, -1.0, -0.0}},
            {{-0.5, 0.0, 0.5}, {0.0, -1.0, -0.0}},
            {{0.5, 0.0, -0.5}, {1.0, 0.0, 0.0}},
            {{0.5, 1.0, 0.5}, {1.0, 0.0, 0.0}},
            {{0.5, 0.0, 0.5}, {1.0, 0.0, 0.0}},
            {{-0.5, 0.0, -0.5}, {0.0, -0.0, -1.0}},
            {{0.5, 1.0, -0.5}, {0.0, -0.0, -1.0}},
            {{0.5, 0.0, -0.5}, {0.0, -0.0, -1.0}},
            {{0.5, 1.0, -0.5}, {0.0, 1.0, -0.0}},
            {{-0.5, 1.0, -0.5}, {0.0, 1.0, -0.0}},
            {{-0.5, 1.0, 0.5}, {0.0, 1.0, -0.0}},
            {{0.5, 0.0, 0.5}, {0.0, 0.0, 1.0}},
            {{0.5, 1.0, 0.5}, {0.0, 0.0, 1.0}},
            {{-0.5, 1.0, 0.5}, {0.0, 0.0, 1.0}},
            {{-0.5, 0.0, 0.5}, {-1.0, -0.0, 0.0}},
            {{-0.5, 1.0, 0.5}, {-1.0, -0.0, 0.0}},
            {{-0.5, 1.0, -0.5}, {-1.0, -0.0, 0.0}},
            {{-0.5, 0.0, -0.5}, {0.0, -1.0, -0.0}},
            {{0.5, 0.0, -0.5}, {0.0, -1.0, -0.0}},
            {{0.5, 0.0, 0.5}, {0.0, -1.0, -0.0}},
            {{0.5, 0.0, -0.5}, {1.0, 0.0, 0.0}},
            {{0.5, 1.0, -0.5}, {1.0, 0.0, 0.0}},
            {{0.5, 1.0, 0.5}, {1.0, 0.0, 0.0}},
            {{-0.5, 0.0, -0.5}, {0.0, 0.0, -1.0}},
            {{-0.5, 1.0, -0.5}, {0.0, 0.0, -1.0}},
            {{0.5, 1.0, -0.5}, {0.0, 0.0, -1.0}},
        });

        planeBuffer = std::make_shared<applesauce::VertexBuffer<Vertex>>(std::initializer_list<Vertex>{
            {{-0.5, 0.0, 0.5}, {-0.0, 1.0, -0.0}},
            {{0.5, 0.0, -0.5}, {-0.0, 1.0, -0.0}},
            {{-0.5, 0.0, -0.5}, {-0.0, 1.0, -0.0}},
            {{-0.5, 0.0, 0.5}, {0.0, 1.0, -0.0}},
            {{0.5, 0.0, 0.5}, {0.0, 1.0, -0.0}},
            {{0.5, 0.0, -0.5}, {0.0, 1.0, -0.0}},
        });

        auto cubeMesh = std::make_shared<applesauce::VertexArray>();
        auto planeMesh = std::make_shared<applesauce::VertexArray>();

        applesauce::VertexBufferDescription desc{
            {applesauce::VertexAttribute::position, 3, offsetof(Vertex, position), sizeof(Vertex)},
            {applesauce::VertexAttribute::normal, 3, offsetof(Vertex, normal), sizeof(Vertex)},
        };

        cubeMesh->addVertexBuffer(*cubeBuffer, desc);
        planeMesh->addVertexBuffer(*planeBuffer, desc);

        meshes.push_back(planeMesh);
        meshes.push_back(cubeMesh);

        glfwSwapInterval(1);

        const auto [width, height] = window.framebufferSize();
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);

        //
        // ****************************
        // *            **            *
        // *            **            *
        // *  **                  **  *
        // *   *      *    *      *   *
        // *   *      *    *      *   *
        // *  **                  **  *
        // *            **            *
        // *            **            *
        // ****************************
    }

    void display() override
    {
        window.clear({0.1f, 0.1f, 0.1f, 1.0f});

        const auto [width, height] = window.framebufferSize();

        glm::mat4 model(1.0f);

        glm::vec4 cameraPos = glm::yawPitchRoll(theta, pitch, 0.0f) * glm::vec4(0, 0, -dist, 0);
        glm::mat4 view = glm::lookAt(glm::vec3(cameraPos),
                                     glm::vec3(0, 0, 0),
                                     glm::vec3(0, 1, 0));
        glm::mat4 projection = glm::perspectiveFov(glm::radians(60.f), static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);
        glm::mat4 MVP = projection * view * model;

        const auto mvp = shader->uniforms()["mMVP"].location;
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));

        meshes[1]->bind();
        glDrawArrays(GL_TRIANGLES, 0, meshes[1]->safeElementCount());
    }

private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<applesauce::VertexBuffer<Vertex>> cubeBuffer;
    std::shared_ptr<applesauce::VertexBuffer<Vertex>> planeBuffer;
    std::vector<std::shared_ptr<applesauce::VertexArray>> meshes;

    float pitch = 0;
    float theta = 0;
    float dist = 8.0f;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}