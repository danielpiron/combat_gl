#pragma once

#include "base64.h"

#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

struct glTF
{
    struct Asset
    {
    public:
        class Version
        {
        public:
            Version(const std::string &version) : versionText(version)
            {
            }
            int major() const
            {
                const auto periodIndex = versionText.find('.');
                return std::stoi(versionText.substr(0, periodIndex));
            }
            int minor() const
            {
                const auto periodIndex = versionText.find('.');
                return std::stoi(versionText.substr(periodIndex + 1));
            }
            operator std::string() const
            {
                return versionText;
            }

        private:
            std::string versionText;
        };

    public:
        Asset() : version("") {}
        Asset(const std::string &version) : version(version) {}

    public:
        Version version;
    };

public:
    struct Accessor
    {
        enum class Type
        {
            NONE = 0,
            SCALAR,
            VEC2,
            VEC3,
            VEC4,
            MAT2,
            MAT3,
            MAT4,
        };

        enum class ComponentType
        {
            NONE = 0,
            BYTE,
            UNSIGNED_BYTE,
            SHORT,
            UNSIGNED_SHORT,
            UNSIGNED_INT,
            FLOAT,
        };

        int bufferView;
        int byteOffset;
        ComponentType componentType;
        bool normalized;
        int count;
        Type type;

        bool operator==(const Accessor &rhs) const
        {
            return bufferView == rhs.bufferView && byteOffset == rhs.byteOffset &&
                   componentType == rhs.componentType && normalized == rhs.normalized &&
                   count == rhs.count && type == rhs.type;
        }

        int componentCount() const
        {
            switch (type)
            {
            case Type::SCALAR:
                return 1;
            case Type::VEC2:
                return 2;
            case Type::VEC3:
                return 3;
            case Type::VEC4:
                return 4;
            case Type::MAT2:
                return 8;
            case Type::MAT3:
                return 9;
            case Type::MAT4:
                return 16;
            default:
                return 0;
            }
        }
    };

    struct Buffer
    {
        std::string uri;
        int byteLength;

        bool operator==(const Buffer &rhs) const
        {
            return uri == rhs.uri && byteLength == rhs.byteLength;
        }

        std::vector<uint8_t> getBytes() const
        {
            std::vector<uint8_t> result(byteLength);
            std::string base64String = uri.substr(uri.find(',') + 1);

            decodeBase64(base64String.c_str(), reinterpret_cast<char *>(&result[0]), base64String.length());
            return result;
        }
    };

    struct BufferView
    {
        enum class Target
        {
            none,
            array_buffer,
            element_array_buffer,
        };

        static Target decodeTarget(int code)
        {
            switch (code)
            {
            case 34962:
                return Target::array_buffer;
            case 34963:
                return Target::element_array_buffer;
            default:
                return Target::none;
            }
        }

        int buffer;
        int byteOffset;
        int byteLength;
        int byteStride;
        Target target;
        std::string name;

        bool operator==(const BufferView &rhs) const
        {
            return buffer == rhs.buffer && byteOffset == rhs.byteOffset &&
                   byteLength == rhs.byteLength && byteStride == rhs.byteStride &&
                   target == rhs.target && name == rhs.name;
        }
    };

    struct Material
    {
        struct PbrMetallicRoughness
        {
            float baseColorFactor[4] = {
                1.0f,
                1.0f,
                1.0f,
                1.0f,
            };
            float metallicFactor = 1.0f;
            float roughnessFactor = 1.0f;

            bool operator==(const PbrMetallicRoughness &rhs) const
            {
                constexpr float EPSILON = 0.00001f;
                const auto d0 = std::fabs(baseColorFactor[0] - rhs.baseColorFactor[0]) < EPSILON;
                const auto d1 = std::fabs(baseColorFactor[1] - rhs.baseColorFactor[1]) < EPSILON;
                const auto d2 = std::fabs(baseColorFactor[2] - rhs.baseColorFactor[2]) < EPSILON;
                const auto d3 = std::fabs(baseColorFactor[3] - rhs.baseColorFactor[3]) < EPSILON;
                const auto dm = std::fabs(metallicFactor - rhs.metallicFactor) < EPSILON;
                const auto dr = std::fabs(roughnessFactor - rhs.roughnessFactor) < EPSILON;

                return d0 && d1 && d2 && d3 && dm && dr;
            }
        };
        std::string alphaMode = "OPAQUE";
        bool doubleSided = false;
        float emissiveFactor[3] = {0, 0, 0};
        std::string name = "";
        PbrMetallicRoughness pbrMetallicRoughness;

        bool operator==(const Material &rhs) const
        {
            constexpr float EPSILON = 0.00001f;
            const auto d0 = std::fabs(emissiveFactor[0] - rhs.emissiveFactor[0]) < EPSILON;
            const auto d1 = std::fabs(emissiveFactor[1] - rhs.emissiveFactor[1]) < EPSILON;
            const auto d2 = std::fabs(emissiveFactor[2] - rhs.emissiveFactor[2]) < EPSILON;
            return alphaMode == rhs.alphaMode && doubleSided == rhs.doubleSided &&
                   name == rhs.name && pbrMetallicRoughness == rhs.pbrMetallicRoughness && d0 && d1 && d2;
        }
    };

    struct Mesh
    {
        struct Primitive
        {
            enum class Mode
            {
                POINTS = 0,
                LINES,
                LINE_LOOP,
                LINE_STRIP,
                TRIANGLES,
                TRIANGLE_STRIP,
                TRIANGLE_FAN
            };

            using Attributes = std::unordered_map<std::string, int>;
            Attributes attributes;
            int indices;
            int material;
            Mode mode;

            bool operator==(const Primitive &rhs) const
            {
                return attributes == rhs.attributes && indices == rhs.indices && material == rhs.material && mode == rhs.mode;
            }
        };

        using Primitives = std::vector<Primitive>;

        std::string name;
        Primitives primitives;

        bool operator==(const Mesh &rhs) const
        {
            return name == rhs.name && primitives == rhs.primitives;
        }
    };

public:
    using Materials = std::vector<Material>;
    using Meshes = std::vector<Mesh>;

    Asset asset;
    std::vector<Accessor> accessors;
    std::vector<BufferView> bufferViews;
    std::vector<Buffer> buffers;
    Materials materials;
    Meshes meshes;
};

extern glTF glTFFromString(const char *jsonText);