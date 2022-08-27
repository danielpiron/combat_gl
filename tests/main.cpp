#include <gtest/gtest.h>
#include <App.h>

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

    // Very subtle error, with an unsubtle error message
    ShaderStageCompile with_errors(
        R"(#version 330 core
           in vec3 vPosition;
           void main() {
            gl_Position = vPosition;
           })");
    ShaderStageCompile with_no_errors(
        R"(#version 330 core
           in vec4 vPosition;
           void main() {
            gl_Position = vPosition;
           })");

    with_errors.run_windowless();
    with_no_errors.run_windowless();

    std::cout << with_no_errors.errorLog << std::endl;
    ASSERT_TRUE(with_no_errors.compileStatus);
    ASSERT_FALSE(with_errors.compileStatus);
    EXPECT_EQ("ERROR: 0:4: Incompatible types (vec4 and vec3) in assignment (and no available implicit conversion)\n", with_errors.errorLog);
}

/*
TEST(App, CanGatherShaderInformation)
{
    class ShaderTestApp : public App
    {
    public:
        void init() override
        {
            const char *vertex_shader_text = R"(
                #version 330 core
                in vec3 vPosition;
                in vec4 vNormal;
                uniform mat4 ModelViewProjection;

                main() {
                    gl_Position = vPosition;
                }
            )";

            const char *fragment_shader_text = R"(
                #version 330 core
                out vec4 fColor;
                void main() {
                    fColor = vec4(0.5, 0.4, 0.8, 1.0);
                }
            )";

            auto shader = create_shader(vertex_shader_text, fragment_shader_text);

            uniforms = shader.uniforms();
            attributes = shader.attributes();
        }

        void display() override
        {
            close();
        }

    public:
        Shader::Uniforms uniforms;
        Shader::Attributes attributes;
    };
}
*/

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}