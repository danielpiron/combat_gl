#define _USE_MATH_DEFINES

#include "applesauce/App.h"
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

struct Mesh
{
    std::shared_ptr<applesauce::VertexArray> vertexArray;
    std::shared_ptr<applesauce::Buffer> indexBuffer;
    int elementCount;
};

Mesh meshFromComponents(const std::vector<glm::vec3> &vertices,
                        const std::vector<glm::vec3> &normals,
                        const std::vector<glm::vec2> &texcoords,
                        const std::vector<uint16_t> &indices)
{
    const int verticesByteCount = sizeof(vertices[0]) * vertices.size();
    const int normalsByteCount = sizeof(normals[0]) * normals.size();
    const int texcoordsByteCount = sizeof(texcoords[0]) * texcoords.size();
    const int indicesByteCount = sizeof(indices[0]) * indices.size();

    auto vertexBuffer = std::make_shared<applesauce::Buffer>(verticesByteCount + normalsByteCount + texcoordsByteCount, applesauce::Buffer::Target::vertex_array);
    auto indexBuffer = std::make_shared<applesauce::Buffer>(indicesByteCount, applesauce::Buffer::Target::element_array);

    { // Set up Vertex Buffer
        vertexBuffer->bind();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(vertexBuffer->map());
        std::memcpy(ptr, &vertices[0], verticesByteCount);
        std::memcpy(ptr + verticesByteCount, &normals[0], normalsByteCount);
        std::memcpy(ptr + verticesByteCount + normalsByteCount, &texcoords[0], texcoordsByteCount);
        vertexBuffer->unmap();
        vertexBuffer->unbind();
    }

    { // Set up IndexBuffer
        indexBuffer->bind();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(indexBuffer->map());
        std::memcpy(ptr, &indices[0], indicesByteCount);
        indexBuffer->unmap();
        indexBuffer->unbind();
    }

    auto vertexArray = std::make_shared<applesauce::VertexArray>();

    applesauce::VertexBufferDescription desc{
        {applesauce::VertexAttribute::position, 3, 0, 0},
        {applesauce::VertexAttribute::normal, 3, verticesByteCount, 0},
        {applesauce::VertexAttribute::texcoord, 2, verticesByteCount + normalsByteCount, 0},
    };

    vertexArray->addVertexBuffer(*vertexBuffer, desc);

    return {vertexArray, indexBuffer, static_cast<int>(indices.size())};
}

Mesh makePlaneMesh(float planeSize)
{
    // Plane
    //
    //  (-0.5, 0, -0.5)   (0.5, 0, -0.5)
    // -Z          2-----3
    //  ^          | \ B |
    //  |          | A \ |
    //  |          0-----1
    //  (-0.5, 0, 0.5)    (0.5, 0, 0.5)
    //      ----> +X
    //
    // Triangle indicies:
    //   A. 0, 1, 2
    //   B. 1, 3, 2

    float halfSize = planeSize / 2.0f;
    std::vector<glm::vec3> vertices{
        {-halfSize, 0, halfSize},  // 0
        {halfSize, 0, halfSize},   // 1
        {-halfSize, 0, -halfSize}, // 2
        {halfSize, 0, -halfSize},  // 3
    };

    // Normals all face "up"
    std::vector<glm::vec3> normals{
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // up
        {0, 1.0f, 0}, // and up
    };

    float uvSize = halfSize;
    std::vector<glm::vec2> texcoords{
        {0, uvSize},      // 0 - Near left
        {uvSize, uvSize}, // 1 - Near right
        {0, 0},           // 2 - Far left
        {uvSize, 0},      // 3 - Far Right
    };

    std::vector<uint16_t> indices{
        0, 1, 2, // Triangle A
        1, 3, 2, // Triangle B
    };
    return meshFromComponents(vertices, normals, texcoords, indices);
}

Mesh makeBoxMesh(float boxSize)
{
    //
    //
    //         6----------7
    //        /|         /|
    //       / |        / |
    //      3----------2  |
    //      |  |       |  |
    //      |  |       |  |
    //      |  5-------|--4
    //      | /        | /
    //      |/         |/
    //      0----------1
    //
    //

    const float hSize = boxSize / 2.0f;
    std::vector<glm::vec3> corner{
        // Near corners
        {-hSize, -hSize, hSize}, // 0
        {hSize, -hSize, hSize},  // 1
        {hSize, hSize, hSize},   // 2
        {-hSize, hSize, hSize},  // 3
        // Far corners
        {hSize, -hSize, -hSize},  // 4
        {-hSize, -hSize, -hSize}, // 5
        {-hSize, hSize, -hSize},  // 6
        {hSize, hSize, -hSize},   // 7
    };

    std::vector<glm::vec3> sideNormal{
        {0, 0, 1.0f},  // near
        {0, 0, -1.0f}, // far
        {-1.0f, 0, 0}, // left
        {1.0f, 0, 0},  // right
        {0, 1.0f, 0},  // top
        {0, -1.0f, 0}, // bottom
    };

    std::vector<glm::vec3> vertices{
        corner[0], corner[1], corner[2], corner[3], // near    0-3
        corner[4], corner[5], corner[6], corner[7], // far     4-7
        corner[5], corner[0], corner[3], corner[6], // left    8-11
        corner[1], corner[4], corner[7], corner[2], // right   12-15
        corner[3], corner[2], corner[7], corner[6], // top     16-19
        corner[1], corner[0], corner[5], corner[4], // bottom  20-23
    };

    std::vector<glm::vec3> normals{
        sideNormal[0], sideNormal[0], sideNormal[0], sideNormal[0], // near
        sideNormal[1], sideNormal[1], sideNormal[1], sideNormal[1], // far
        sideNormal[2], sideNormal[2], sideNormal[2], sideNormal[2], // left
        sideNormal[3], sideNormal[3], sideNormal[3], sideNormal[3], // right
        sideNormal[4], sideNormal[4], sideNormal[4], sideNormal[4], // top
        sideNormal[5], sideNormal[5], sideNormal[5], sideNormal[5], // bottom
    };

    std::vector<glm::vec2> texcoords{
        // near
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // far
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // left
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // right
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // top
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
        // bottom
        {0, 1},
        {1, 1},
        {1, 0},
        {0, 0},
    };

    std::vector<uint16_t> indices{
        // near
        0,
        1,
        2,
        0,
        2,
        3,
        // far
        4,
        5,
        6,
        4,
        6,
        7,
        // left
        8,
        9,
        10,
        8,
        10,
        11,
        // right
        12,
        13,
        14,
        12,
        14,
        15,
        // top
        16,
        17,
        18,
        16,
        18,
        19,
        // bottom
        20,
        21,
        22,
        20,
        22,
        23,
    };

    return meshFromComponents(vertices, normals, texcoords, indices);
}

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
    }

    void display() override
    {
        glm::vec3 lightDir = glm::normalize(glm::vec3{0.5, 1, 0.25});

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
                model *= glm::mat4(entity.orientation);

                glm::mat4 MVPMatrix = lightSpaceMatrix * model;

                shadow->set("MVPMatrix", MVPMatrix);

                auto &mesh = entity.mesh;
                mesh->vertexArray->bind();
                mesh->indexBuffer->bindTo(applesauce::Buffer::Target::element_array);
                glDrawElements(GL_TRIANGLES, mesh->elementCount, GL_UNSIGNED_SHORT, reinterpret_cast<void *>(0));
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
        glm::mat4 view = camera.lookAtMatrix(glm::vec3{0});
        glm::mat4 projection = camera.projectionMatrix();

        glm::vec3 LightDirection = glm::mat3(view) * lightDir;

        for (const auto &entity : entities)
        {
            auto model = glm::mat4(1.0);
            model = glm::translate(model, entity.position);
            model *= glm::mat4(entity.orientation);

            glm::mat4 modelView = view * model;
            glm::mat3 normalMatrix = glm::mat3(modelView);

            glm::mat4 MVPMatrix = projection * modelView;
            glm::mat4 LightViewMatrix = lightSpaceMatrix * model;

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