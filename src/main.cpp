#define _USE_MATH_DEFINES

#include "applesauce/App.h"
#include "applesauce/AltMesh.h"
#include "applesauce/VertexBuffer.h"
#include "applesauce/VertexArray.h"
#include "applesauce/Shader.h"
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

#include <png.h>

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <list>
#include <cmath>
#include <memory>
#include <ostream>
#include <sstream>
#include <vector>

static constexpr unsigned int SHADOW_WIDTH = 1024,
                              SHADOW_HEIGHT = 1024;

GLuint try_png(const char *filename)
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
            GLuint texture = 0;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            std::cout << "Texture Dimensions: " << image.width << "x" << image.height << std::endl;

            auto ptr = reinterpret_cast<GLubyte *>(buffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(image.width),
                         static_cast<GLsizei>(image.height), 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr);

            std::cout << "Texture loaded" << std::endl;

            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            return texture;
        }
    }
    return 0;
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
        glm::quat orientation;
        std::shared_ptr<Mesh> mesh;
        GLuint textureId;
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

        checkerTexture = try_png("assets/textures/Checker.png");
        whiteSquareTexture = try_png("assets/textures/White Square.png");

        camera.fieldOfVision = 60.0f;

        auto box = std::make_shared<Mesh>(makeBoxMesh(1.0f));
        auto plane = std::make_shared<Mesh>(makePlaneMesh(20));

        entities.push_back({glm::vec3{0}, glm::quat{}, plane, checkerTexture});

        int numberOfObjects = 20;
        float radius = 5.0f;

        for (int i = 0; i < numberOfObjects; i++)
        {
            float angle = i * M_PI * 2 / numberOfObjects;
            float x = cosf(angle) * radius;
            float z = sinf(angle) * radius;
            glm::vec3 position{x, 0.5, z};

            glm::quat orientation{glm::vec3{0, -angle, 0}};
            entities.push_back({position, orientation, box, whiteSquareTexture});
        }

        // Init shadow mapping bits

        glGenFramebuffers(1, &depthMapFBO);

        glGenTextures(1, &depthMap);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                     SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

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

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        static float lightDist = 11.2;
        static float lightSize = 15.0f;
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
                model *= glm::mat4(entity.orientation);

                glm::mat4 MVPMatrix = lightSpaceMatrix * model;

                shadow->set("MVPMatrix", MVPMatrix);

                auto &mesh = entity.mesh;
                mesh->vertexArray->bind();
                mesh->indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                glDrawElements(GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
            }
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
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

        // This should have a translate of 0.5 in each coordinate, but instead scale is influencing,
        // so some ordering is wrong. The 1.0f translate is a hacky fix.
        glm::mat4 shadowMatrix = glm::translate(glm::scale(glm::mat4{1.0f}, glm::vec3{0.5f}), glm::vec3{1.0f});

        shadowMatrix *= lightSpaceMatrix;

        for (const auto &entity : entities)
        {
            auto model = glm::mat4(1.0);
            model = glm::translate(model, entity.position);
            model *= glm::mat4(entity.orientation);

            glm::mat4 modelView = view * model;
            glm::mat3 normalMatrix = glm::mat3(modelView);

            glm::mat4 MVPMatrix = projection * modelView;
            glm::mat4 LightViewMatrix = shadowMatrix * model;

            shader->set("MVPMatrix", MVPMatrix);
            shader->set("ModelViewMatrix", modelView);
            shader->set("LightViewMatrix", LightViewMatrix);
            shader->set("NormalMatrix", normalMatrix);
            shader->set("Color", glm::vec3(0.5, 0.5, 0.5));
            shader->set("Ambient", ambient);
            shader->set("LightColor", glm::vec3{1.0, 1.0, 1.0});
            shader->set("LightDirection", LightDirection);
            shader->set("SpecularPower", specularPower);
            shader->set("SpecularStrength", specularStrength);

            shader->set("shadowMap", 0);
            shader->set("albedo", 1);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, entity.textureId);

            auto &mesh = entity.mesh;
            mesh->vertexArray->bind();
            mesh->indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
            glDrawElements(GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
        }
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

    Camera camera;
    float pitch = 0.955591;
    float theta = 2.90973;
    float dist = 10.1983;

    double last_xpos = 0;
    double last_ypos = 0;
    bool move_camera = false;

    glm::vec3 ambient{0.3, 0.3, 0.3};

    float specularPower = 32.0f;
    float specularStrength = 1.0f;

    GLuint checkerTexture;
    GLuint whiteSquareTexture;

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