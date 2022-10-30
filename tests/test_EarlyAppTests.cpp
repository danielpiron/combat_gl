#include <gtest/gtest.h>

#include <applesauce/App.h>

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <ostream>
#include <string>

std::ostream &operator<<(std::ostream &os, const Shader::Attribute &attrib)
{
    os << '{'
       << "Location: " << attrib.location << ", "
       << "Type: " << attrib.type << ", "
       << "Size: " << attrib.size
       << '}';
    return os;
}

std::ostream &
operator<<(std::ostream &os, const Shader::Uniform &uniform)
{
    os << '{'
       << "Location: " << uniform.location << ", "
       << "Type: " << uniform.type << ", "
       << "Size: " << uniform.size
       << '}';
    return os;
}

TEST(App, CanRunWindowLess)
{

    class TestApp : public App
    {
        void init() override
        {
        }
        void display() override
        {
            close();
        }
    };

    TestApp testApp;
    testApp.run_windowless();
}

TEST(App, CanReportShaderStageCompileErrors)
{
    class ShaderStageCompile : public App
    {
    public:
        ShaderStageCompile(const char *source) : source(source) {}
        void init() override
        {
            auto stage = Shader::Stage(source, Shader::Stage::Type::vertex);

            compileStatus = stage.compile();
            if (!compileStatus)
            {
                errorLog = stage.error_log();
            }
        }
        void display() override
        {
            close();
        }

    private:
        std::string source;

    public:
        bool compileStatus;
        std::string errorLog;
    };

    {
        ShaderStageCompile with_errors(
            R"(#version 330 core
           in vec3 vPosition;
           void main() {
            gl_Position = vPosition;
           })");

        with_errors.run_windowless();

        ASSERT_FALSE(with_errors.compileStatus);
        EXPECT_EQ("ERROR: 0:4: Incompatible types (vec4 and vec3) in assignment (and no available implicit conversion)\n", with_errors.errorLog);
    }

    {
        ShaderStageCompile with_no_errors(
            R"(#version 330 core
           in vec4 vPosition;
           void main() {
            gl_Position = vPosition;
           })");

        with_no_errors.run_windowless();
        EXPECT_TRUE(with_no_errors.compileStatus);
    }
}

TEST(App, CanReportShaderCompileErrors)
{
    class ShaderCompileTest : public App
    {
    public:
        ShaderCompileTest(const char *vs_source, const char *fs_source) : vs_source(vs_source), fs_source(fs_source) {}
        void init() override
        {
            Shader shader;

            shader.add_vertex_stage(vs_source);
            shader.add_fragment_stage(fs_source);

            compileStatus = shader.compile_and_link();

            if (!compileStatus)
            {
                errorLog = shader.error_log();
            }
        }
        void display() override
        {
            close();
        }

    private:
        std::string vs_source;
        std::string fs_source;

    public:
        bool compileStatus;
        std::string errorLog;
    };

    const char *vs_with_errors =
        R"(#version 330 core
           in vec3 vPosition;
           void main() {
            gl_Position = vPosition;
           })";

    const char *vs_without_errors =
        R"(#version 330 core
           in vec4 vPosition;
           void main() {
            gl_Position = vPosition;
           })";

    const char *fs_with_errors = R"(
        out vec4 fColor;
        void main() {
            fColor = vec4(0.5, 0.4, 0.8, 1.0);
        }
    )";
    const char *fs_without_errors = R"(
        #version 330 core
        out vec4 fColor;
        void main() {
            fColor = vec4(0.5, 0.4, 0.8, 1.0);
        }
    )";
    const char *fs_with_mismatch = R"(
        #version 330 core
        in vec4 normal;
        out vec4 fColor;
        void main() {
            fColor = normal;
        }
    )";

    {
        ShaderCompileTest with_vs_errors(vs_with_errors, fs_without_errors);
        with_vs_errors.run_windowless();

        EXPECT_FALSE(with_vs_errors.compileStatus);
        EXPECT_EQ("VERTEX SHADER ERROR: 0:4: Incompatible types (vec4 and vec3) in assignment (and no available implicit conversion)\n", with_vs_errors.errorLog);
    }

    {
        ShaderCompileTest with_fs_errors(vs_without_errors, fs_with_errors);
        with_fs_errors.run_windowless();

        EXPECT_FALSE(with_fs_errors.compileStatus);
        EXPECT_EQ("FRAGMENT SHADER ERROR: 0:2: '' :  #version required and missing.\n", with_fs_errors.errorLog);
    }

    {
        ShaderCompileTest with_link_errors(vs_without_errors, fs_with_mismatch);
        with_link_errors.run_windowless();

        EXPECT_FALSE(with_link_errors.compileStatus);
        EXPECT_EQ("LINKING ERROR: Input of fragment shader 'normal' not written by vertex shader\n", with_link_errors.errorLog);
    }

    {
        ShaderCompileTest with_no_errors(vs_without_errors, fs_without_errors);
        with_no_errors.run_windowless();

        EXPECT_TRUE(with_no_errors.compileStatus);
        EXPECT_EQ("", with_no_errors.errorLog);
    }
}

TEST(App, CanAcquireShaderAttributes)
{
    class ShaderAttributesTest : public App
    {
    public:
        ShaderAttributesTest(const char *vs_source, const char *fs_source) : vs_source(vs_source), fs_source(fs_source) {}
        void init() override
        {
            Shader shader;

            shader.add_vertex_stage(vs_source);
            shader.add_fragment_stage(fs_source);

            compileStatus = shader.compile_and_link();

            if (!compileStatus)
            {
                errorLog = shader.error_log();
            }
            else
            {
                attributes = shader.attributes();
            }
        }
        void display() override
        {
            close();
        }

    private:
        std::string vs_source;
        std::string fs_source;

    public:
        bool compileStatus = false;
        std::string errorLog;
        Shader::Attributes attributes;
    };

    const char *vs_source =
        R"(#version 330 core
           in vec4 vPosition;
           in vec3 vNormal;
           in mat4 mModel;
           in float zRandom[4];

           out vec3 normal;

           void main() {
            gl_Position = vPosition * mModel;
            normal = vNormal * zRandom[2];
           })";

    const char *fs_source = R"(
        #version 330 core
        in vec3 normal;
        out vec4 fColor;
        void main() {
            fColor = vec4(0.5, 0.4, 0.8, 1.0);
        }
    )";

    Shader::Attributes expected = {
        {"zRandom", {0, GL_FLOAT, 4}},
        {"mModel", {4, GL_FLOAT_MAT4, 1}},
        {"vNormal", {8, GL_FLOAT_VEC3, 1}},
        {"vPosition", {9, GL_FLOAT_VEC4, 1}},
    };

    ShaderAttributesTest collect_attributes(vs_source, fs_source);
    collect_attributes.run_windowless();

    ASSERT_TRUE(collect_attributes.compileStatus);
    EXPECT_EQ(expected, collect_attributes.attributes);
}

TEST(App, CanAcquireShaderUniforms)
{
    class ShaderUniformsTest : public App
    {
    public:
        ShaderUniformsTest(const char *vs_source, const char *fs_source) : vs_source(vs_source), fs_source(fs_source) {}
        void init() override
        {
            Shader shader;

            shader.add_vertex_stage(vs_source);
            shader.add_fragment_stage(fs_source);

            compileStatus = shader.compile_and_link();

            if (!compileStatus)
            {
                errorLog = shader.error_log();
            }
            else
            {
                uniforms = shader.uniforms();
            }
        }
        void display() override
        {
            close();
        }

    private:
        std::string vs_source;
        std::string fs_source;

    public:
        bool compileStatus = false;
        std::string errorLog;
        Shader::Uniforms uniforms;
    };

    const char *vs_source =
        R"(#version 330 core
           in vec4 vPosition;
           in vec3 vNormal;

           uniform mat4 ModelViewProjection;
           uniform mat4 ModelView;

           out vec3 normal;

           void main() {
            gl_Position = vPosition * ModelViewProjection;
            normal = (vec4(vNormal, 0) * ModelView).xyz;
           })";

    const char *fs_source = R"(
        #version 330 core

        uniform vec3 LightDirection;
        in vec3 normal;
        out vec4 fColor;

        void main() {
            float factor = dot(normal, LightDirection);
            fColor = vec4(0.5, 0.4, 0.8, 1.0) * factor;
        }
    )";

    Shader::Uniforms expected = {
        {"ModelViewProjection", {0, GL_FLOAT_MAT4, 1}},
        {"ModelView", {4, GL_FLOAT_MAT4, 1}},
        {"LightDirection", {8, GL_FLOAT_VEC3, 1}},
    };

    ShaderUniformsTest collect_uniforms(vs_source, fs_source);
    collect_uniforms.run_windowless();

    EXPECT_TRUE(collect_uniforms.compileStatus);
    ASSERT_EQ("", collect_uniforms.errorLog);

    EXPECT_EQ(expected, collect_uniforms.uniforms);
}

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

bool operator==(const Vertex &lhs, const Vertex &rhs)
{
    return lhs.position == rhs.position && lhs.normal == rhs.normal;
}