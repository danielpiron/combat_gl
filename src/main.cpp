#include "App.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>

struct Vertex
{
    glm::vec4 color;
    glm::vec2 position;
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
           in vec4 vPosition;
           in vec4 vColor;

           out vec4 color;
           void main() {
            gl_Position = vPosition;
            color = vColor;
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
            {{1.0, 0.0, 0.0, 1.0}, {-0.90, -0.90}},
            {{1.0, 1.0, 0.0, 1.0}, {0.85, -0.90}},
            {{1.0, 0.0, 1.0, 1.0}, {-0.90, 0.85}},
            {{1.0, 0.0, 0.0, 1.0}, {0.90, -0.85}},
            {{0.0, 1.0, 0.0, 1.0}, {0.90, 0.90}},
            {{0.0, 0.0, 1.0, 1.0}, {-0.85, 0.90}},
        });

        glUseProgram(shader->glId());

        glGenVertexArrays(1, &vao_triangles);
        glBindVertexArray(vao_triangles);

        auto pos = shader->attributes()["vPosition"].location;
        auto col = shader->attributes()["vColor"].location;

        glBindBuffer(GL_ARRAY_BUFFER, buffer->glId());

        VERTEX_PROPERTY(col, Vertex, color);
        VERTEX_PROPERTY(pos, Vertex, position);

        glEnableVertexAttribArray(pos);
        glEnableVertexAttribArray(col);

        glfwSwapInterval(1);

        const auto [width, height] = renderer.framebuffer_size();
        glViewport(0, 0, width, height);
    }

    void display() override
    {
        renderer.clear({0.1f, 0.1f, 0.1f, 1.0f});

        glBindVertexArray(vao_triangles);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

private:
    GLuint vao_triangles = 0;
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Buffer<Vertex>> buffer;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}