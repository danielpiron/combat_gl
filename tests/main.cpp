#include <gtest/gtest.h>
#include <App.h>

#include <string>

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

    ShaderCompileTest with_vs_errors(vs_with_errors, fs_without_errors);
    ShaderCompileTest with_fs_errors(vs_without_errors, fs_with_errors);
    ShaderCompileTest with_link_errors(vs_without_errors, fs_with_mismatch);
    ShaderCompileTest with_no_errors(vs_without_errors, fs_without_errors);

    with_vs_errors.run_windowless();
    with_fs_errors.run_windowless();
    with_link_errors.run_windowless();
    with_no_errors.run_windowless();

    EXPECT_FALSE(with_vs_errors.compileStatus);
    EXPECT_EQ("VERTEX SHADER ERROR: 0:4: Incompatible types (vec4 and vec3) in assignment (and no available implicit conversion)\n", with_vs_errors.errorLog);

    EXPECT_FALSE(with_fs_errors.compileStatus);
    EXPECT_EQ("FRAGMENT SHADER ERROR: 0:2: '' :  #version required and missing.\n", with_fs_errors.errorLog);

    EXPECT_FALSE(with_link_errors.compileStatus);
    EXPECT_EQ("LINKING ERROR: Input of fragment shader 'normal' not written by vertex shader\n", with_link_errors.errorLog);

    EXPECT_TRUE(with_no_errors.compileStatus);
    EXPECT_EQ("", with_no_errors.errorLog);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}