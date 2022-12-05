#include "gltf.h"

#include <nlohmann/json.hpp>

void from_json(const nlohmann::json &j, glTF::Accessor::ComponentType &ct)
{
    ct = glTF::Accessor::ComponentType::NONE;
    switch (j.get<int>())
    {
    case 5120:
        ct = glTF::Accessor::ComponentType::BYTE;
        break;
    case 5121:
        ct = glTF::Accessor::ComponentType::UNSIGNED_BYTE;
        break;
    case 5122:
        ct = glTF::Accessor::ComponentType::SHORT;
        break;
    case 5123:
        ct = glTF::Accessor::ComponentType::UNSIGNED_SHORT;
        break;
    case 5125:
        ct = glTF::Accessor::ComponentType::UNSIGNED_INT;
        break;
    case 5126:
        ct = glTF::Accessor::ComponentType::FLOAT;
        break;
    }
}

void from_json(const nlohmann::json &j, glTF::Accessor::Type &t)
{
    t = glTF::Accessor::Type::NONE;

    const auto typeText = j.get<std::string>();
    if (typeText == "SCALAR")
        t = glTF::Accessor::Type::SCALAR;
    else if (typeText == "VEC2")
        t = glTF::Accessor::Type::VEC2;
    else if (typeText == "VEC3")
        t = glTF::Accessor::Type::VEC3;
    else if (typeText == "VEC4")
        t = glTF::Accessor::Type::VEC4;
    else if (typeText == "MAT2")
        t = glTF::Accessor::Type::MAT2;
    else if (typeText == "MAT3")
        t = glTF::Accessor::Type::MAT3;
    else if (typeText == "MAT4")
        t = glTF::Accessor::Type::MAT4;
}

void from_json(const nlohmann::json &j, glTF::Accessor &a)
{
    j.at("componentType").get_to(a.componentType);
    j.at("count").get_to(a.count);
    j.at("type").get_to(a.type);

    if (j.count("bufferView"))
        j.at("bufferView").get_to(a.bufferView);

    if (j.count("byteOffset"))
        j.at("byteOffset").get_to(a.byteOffset);

    if (j.count("normalized"))
        j.at("normalized").get_to(a.normalized);
}

void from_json(const nlohmann::json &j, glTF::Asset::Version &v)
{
    v = glTF::Asset::Version(j.get<std::string>());
}

void from_json(const nlohmann::json &j, glTF::Asset &a)
{
    j.at("version").get_to(a.version);
}

void from_json(const nlohmann::json &j, glTF::Buffer &b)
{
    j.at("uri").get_to(b.uri);
    j.at("byteLength").get_to(b.byteLength);
}

void from_json(const nlohmann::json &j, glTF::BufferView &bv)
{
    j.at("buffer").get_to(bv.buffer);
    j.at("byteLength").get_to(bv.byteLength);

    if (j.count("byteOffset"))
        j.at("byteOffset").get_to(bv.byteOffset);

    if (j.count("byteStride"))
        j.at("byteStride").get_to(bv.byteStride);

    if (j.count("name"))
        j.at("name").get_to(bv.name);

    if (j.count("target"))
        bv.target = glTF::BufferView::decodeTarget(j.at("target").get<int>());
}

void from_json(const nlohmann::json &j, glTF::Mesh::Primitive &mp)
{
    j.at("attributes").get_to(mp.attributes);
    j.at("indices").get_to(mp.indices);

    if (j.count("material"))
        j.at("material").get_to(mp.material);

    if (j.count("mode"))
        j.at("mode").get_to(mp.mode);
    else
        mp.mode = glTF::Mesh::Primitive::Mode::TRIANGLES;
}

void from_json(const nlohmann::json &j, glTF::Mesh &m)
{
    j.at("name").get_to(m.name);
    j.at("primitives").get_to(m.primitives);
}

void from_json(const nlohmann::json &j, glTF &gltf)
{
    j.at("asset").get_to(gltf.asset);
    if (j.count("accessors"))
    {
        j.at("accessors").get_to(gltf.accessors);
    }
    if (j.count("bufferViews"))
    {
        j.at("bufferViews").get_to(gltf.bufferViews);
    }
    if (j.count("buffers"))
    {
        j.at("buffers").get_to(gltf.buffers);
    }
    if (j.count("meshes"))
    {
        j.at("meshes").get_to(gltf.meshes);
    }
}

glTF glTFFromString(const char *jsonText)
{
    glTF gltf;
    auto j = nlohmann::json::parse(jsonText);
    j.get_to(gltf);
    return gltf;
}
