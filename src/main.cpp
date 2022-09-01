#include "App.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <memory>

struct Vertex
{
    glm::vec4 color;
    glm::vec2 position;
};

static const Vertex vertices[] = {
    {{1.0, 0.0, 0.0, 1.0}, {-0.90, -0.90}},
    {{1.0, 1.0, 0.0, 1.0}, {0.85, -0.90}},
    {{1.0, 0.0, 1.0, 1.0}, {-0.90, 0.85}},
    {{1.0, 0.0, 0.0, 1.0}, {0.90, -0.85}},
    {{0.0, 1.0, 0.0, 1.0}, {0.90, 0.90}},
    {{0.0, 0.0, 1.0, 1.0}, {-0.85, 0.90}},
};

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

        glUseProgram(shader->glId());

        glGenBuffers(1, &buf_triangles);
        glBindBuffer(GL_ARRAY_BUFFER, buf_triangles);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glGenVertexArrays(1, &vao_triangles);
        glBindVertexArray(vao_triangles);

        auto pos = shader->attributes()["vPosition"].location;
        auto col = shader->attributes()["vColor"].location;

        glBindBuffer(GL_ARRAY_BUFFER, buf_triangles);
        glVertexAttribPointer(pos,
                              sizeof(decltype(Vertex::position)) / sizeof(typename decltype(vertices[0].position)::value_type),
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, position)));
        glEnableVertexAttribArray(pos);

        glVertexAttribPointer(col,
                              sizeof(vertices[0].color) / sizeof(typename decltype(vertices[0].color)::value_type),
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              reinterpret_cast<void *>(offsetof(Vertex, color)));
        glEnableVertexAttribArray(col);
    }

    void display() override
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(vao_triangles);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / sizeof(vertices[0]));
    }

private:
    GLuint buf_triangles = 0;
    // GLuint buf_colors = 0;
    GLuint vao_triangles = 0;
    std::shared_ptr<Shader> shader;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}