#include "Shader.h"

#include <filesystem>
#include <fstream>

static std::string readFileText(const char *filename)
{
    std::ifstream f{filename};
    return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
}

std::shared_ptr<Shader> loadShader(const char *name)
{
    static const std::string vertexShaderExt = ".vs.glsl";
    static const std::string fragmentShaderExt = ".fs.glsl";

    std::filesystem::path assetsPath = "assets/shaders";
    std::filesystem::path vertexShaderPath = assetsPath / (std::string(name) + vertexShaderExt);
    std::filesystem::path fragmentShaderPath = assetsPath / (std::string(name) + fragmentShaderExt);

    const auto vertex_shader_text = readFileText(vertexShaderPath.string().c_str());
    const auto fragment_shader_text = readFileText(fragmentShaderPath.string().c_str());

    auto shader = std::make_shared<Shader>();
    shader->add_vertex_stage(vertex_shader_text);
    shader->add_fragment_stage(fragment_shader_text);

    if (!shader->compile_and_link())
    {
        return nullptr;
    }

    return shader;
}
