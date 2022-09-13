#include "App.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>

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

class Triangles : public App
{
public:
    void init() override
    {
        const char *vertex_shader_text = R"(
           #version 330 core
           in vec3 vPosition;
           in vec3 vNormal;

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

        buffer = std::make_shared<Buffer<Vertex>>(std::initializer_list<Vertex>{
            {{1.0, 1.0, 1.0}, {0.0, -0.0, 1.0}},
            {{-1.0, -1.0, 1.0}, {0.0, -0.0, 1.0}},
            {{1.0, -1.0, 1.0}, {0.0, -0.0, 1.0}},
            {{1.0, -1.0, -1.0}, {0.0, -1.0, 0.0}},
            {{-1.0, -1.0, 1.0}, {0.0, -1.0, 0.0}},
            {{-1.0, -1.0, -1.0}, {0.0, -1.0, 0.0}},
            {{-1.0, -1.0, -1.0}, {-1.0, -0.0, 0.0}},
            {{-1.0, 1.0, 1.0}, {-1.0, -0.0, 0.0}},
            {{-1.0, 1.0, -1.0}, {-1.0, -0.0, 0.0}},
            {{-1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{-1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{1.0, 1.0, -1.0}, {1.0, -0.0, 0.0}},
            {{1.0, -1.0, 1.0}, {1.0, -0.0, 0.0}},
            {{1.0, -1.0, -1.0}, {1.0, -0.0, 0.0}},
            {{-1.0, 1.0, -1.0}, {0.0, 1.0, -0.0}},
            {{1.0, 1.0, 1.0}, {0.0, 1.0, -0.0}},
            {{1.0, 1.0, -1.0}, {0.0, 1.0, -0.0}},
            {{1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}},
            {{-1.0, 1.0, 1.0}, {0.0, 0.0, 1.0}},
            {{-1.0, -1.0, 1.0}, {0.0, 0.0, 1.0}},
            {{1.0, -1.0, -1.0}, {0.0, -1.0, 0.0}},
            {{1.0, -1.0, 1.0}, {0.0, -1.0, 0.0}},
            {{-1.0, -1.0, 1.0}, {0.0, -1.0, 0.0}},
            {{-1.0, -1.0, -1.0}, {-1.0, -0.0, -0.0}},
            {{-1.0, -1.0, 1.0}, {-1.0, -0.0, -0.0}},
            {{-1.0, 1.0, 1.0}, {-1.0, -0.0, -0.0}},
            {{-1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{1.0, 1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{1.0, -1.0, -1.0}, {0.0, 0.0, -1.0}},
            {{1.0, 1.0, -1.0}, {1.0, -0.0, 0.0}},
            {{1.0, 1.0, 1.0}, {1.0, -0.0, 0.0}},
            {{1.0, -1.0, 1.0}, {1.0, -0.0, 0.0}},
            {{-1.0, 1.0, -1.0}, {0.0, 1.0, 0.0}},
            {{-1.0, 1.0, 1.0}, {0.0, 1.0, 0.0}},
            {{1.0, 1.0, 1.0}, {0.0, 1.0, 0.0}},
        });

        glUseProgram(shader->glId());

        glGenVertexArrays(1, &vao_triangles);
        glBindVertexArray(vao_triangles);

        const auto pos = shader->attributes()["vPosition"].location;
        const auto norm = shader->attributes()["vNormal"].location;

        glBindBuffer(GL_ARRAY_BUFFER, buffer->glId());

        std::cout << "Vertex Size: " << sizeof(Vertex) << std::endl;
        std::cout << "Vertex Position: " << sizeof(decltype(Vertex::position)) << std::endl;
        std::cout << "Vertex Normal: " << sizeof(decltype(Vertex::normal)) << std::endl;
        std::cout << "Offset of Position: " << offsetof(Vertex, position) << std::endl;
        std::cout << "Offset of Normal: " << offsetof(Vertex, normal) << std::endl;
        VERTEX_PROPERTY(pos, Vertex, position);
        VERTEX_PROPERTY(norm, Vertex, normal);

        glEnableVertexAttribArray(pos);
        glEnableVertexAttribArray(norm);

        glfwSwapInterval(1);

        const auto [width, height] = renderer.framebuffer_size();
        glViewport(0, 0, width, height);
        glEnable(GL_DEPTH_TEST);
    }

    void display() override
    {
        renderer.clear({0.1f, 0.1f, 0.1f, 1.0f});

        const auto [width, height] = renderer.framebuffer_size();

        glm::mat4 model(1.0f);
        model = glm::rotate(model, theta, glm::vec3(0, 1.0f, 0));

        glm::mat4 view(1.0f);
        view = glm::rotate(view, glm::radians(15.f), glm::vec3(1.0, 0, 0));
        view = glm::translate(view, glm::vec3(0, -2.0f, -8.0f));
        glm::mat4 projection = glm::perspectiveFov(glm::radians(60.f), static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);

        glm::mat4 MVP = projection * view * model;

        const auto mvp = shader->uniforms()["mMVP"].location;
        glUniformMatrix4fv(mvp, 1, GL_FALSE, glm::value_ptr(MVP));

        glBindVertexArray(vao_triangles);
        glDrawArrays(GL_TRIANGLES, 0, buffer->size());

        theta += 0.01f;
    }

private:
    GLuint vao_triangles = 0;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Buffer<Vertex>> buffer;
    float theta = 0;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}