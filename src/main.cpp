#include "App.h"

#include <glm/vec2.hpp>
#include <memory>

static const glm::vec2 vertices[] = {
    {-0.90, -0.90},
    {0.85, -0.90},
    {-0.90, 0.85},
    {0.90, -0.85},
    {0.90, 0.90},
    {-0.85, 0.90},
};

class Triangles : public App
{
public:
    void init() override
    {
        const char *vertex_shader_text = R"(
           #version 330 core
           in vec4 vPosition;
           void main() {
            gl_Position = vPosition;
           })";

        const char *fragment_shader_text = R"(
        #version 330 core
        out vec4 fColor;
        void main() {
            fColor = vec4(0.5, 0.4, 0.8, 1.0);
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
        glBindBuffer(GL_ARRAY_BUFFER, buf_triangles);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
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
    GLuint vao_triangles = 0;
    std::shared_ptr<Shader> shader;
};

int main()
{
    Triangles app;
    app.run();
    return 0;
}