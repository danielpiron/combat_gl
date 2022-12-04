#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <util/base64.h>

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

	struct Mesh
	{
		struct Primitive
		{
			using Attributes = std::unordered_map<std::string, int>;
			Attributes attributes;
			int indices;
			int material;

			bool operator==(const Primitive &rhs) const
			{
				return attributes == rhs.attributes && indices == rhs.indices && material == rhs.material;
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
	using Meshes = std::vector<Mesh>;

	Asset asset;
	std::vector<Accessor> accessors;
	std::vector<BufferView> bufferViews;
	std::vector<Buffer> buffers;
	Meshes meshes;
};

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
	j.at("material").get_to(mp.material);
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

TEST(glTFLoader, CanParseAssetVersion)
{
	const char *asset_version_only = R"({ "asset": { "version": "2.0" } })";
	auto gltf = glTFFromString(asset_version_only);

	EXPECT_EQ(std::string("2.0"), static_cast<std::string>(gltf.asset.version));
	EXPECT_EQ(2, gltf.asset.version.major());
	EXPECT_EQ(0, gltf.asset.version.minor());
}

TEST(glTFLoader, CanParseBufferViews)
{
	const char *bufferViews = R"({
			"asset": { "version": "2.0" }, 
			"bufferViews": [
			  {
				"buffer": 0,
				"byteLength": 6,
				"target": 34963
              },
			  {
			    "buffer": 0,
			    "byteOffset": 8,
			    "byteLength": 36,
				"byteStride": 12,
			    "target": 34962,
				"name": "test"
			  }
		    ]
		  })";
	auto gltf = glTFFromString(bufferViews);

	glTF::BufferView firstView{0, 0, 6, 0, glTF::BufferView::Target::element_array_buffer, ""};
	glTF::BufferView secondView{0, 8, 36, 12, glTF::BufferView::Target::array_buffer, "test"};

	ASSERT_EQ(2, gltf.bufferViews.size());
	EXPECT_EQ(firstView, gltf.bufferViews[0]);
	EXPECT_EQ(secondView, gltf.bufferViews[1]);
}

TEST(glTFLoader, CanParseBuffers)
{
	const char *buffers = R"({
			"asset": { "version": "2.0" }, 
			"buffers" : [
				{
				"uri" : "data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=",
				"byteLength" : 44
				}
			]
		  })";
	auto gltf = glTFFromString(buffers);

	glTF::Buffer expectedBuffer{"data:application/octet-stream;base64,AAABAAIAAAAAAAAAAAAAAAAAAAAAAIA/AAAAAAAAAAAAAAAAAACAPwAAAAA=", 44};
	std::vector<uint8_t> expectedRawBuffer{
		0x00,
		0x00,
		0x01,
		0x00,
		0x02,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x80,
		0x3f,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x80,
		0x3f,
		0x00,
		0x00,
		0x00,
		0x00};

	ASSERT_EQ(1, gltf.buffers.size());
	EXPECT_EQ(expectedBuffer, gltf.buffers[0]);
	EXPECT_EQ(expectedRawBuffer, gltf.buffers[0].getBytes());
}

TEST(glTFLoader, CanParseAccessor)
{
	const char *accessors = R"({
			"asset": { "version": "2.0" },
			"accessors" : [
				{
				"bufferView" : 0,
				"byteOffset" : 0,
				"componentType" : 5123,
				"count" : 3,
				"type" : "SCALAR",
				"max" : [ 2 ],
				"min" : [ 0 ]
				},
				{
				"bufferView" : 1,
				"byteOffset" : 0,
				"componentType" : 5126,
				"count" : 3,
				"type" : "VEC3",
				"max" : [ 1.0, 1.0, 0.0 ],
				"min" : [ 0.0, 0.0, 0.0 ]
				}
			]
		  })";

	auto gltf = glTFFromString(accessors);

	std::vector<glTF::Accessor> expectedAccessors{
		{0, 0, glTF::Accessor::ComponentType::UNSIGNED_SHORT, false, 3, glTF::Accessor::Type::SCALAR},
		{1, 0, glTF::Accessor::ComponentType::FLOAT, false, 3, glTF::Accessor::Type::VEC3},
	};

	EXPECT_EQ(expectedAccessors, gltf.accessors);
}

TEST(glTFLoader, CanParseMeshes)
{
	const char *meshes = R"({
		"asset": { "version": "2.0" },
		"meshes" : [
				{
					"name" : "Cube",
					"primitives" : [
						{
							"attributes" : {
								"POSITION" : 0,
								"NORMAL" : 1,
								"TEXCOORD_0" : 2
							},
							"indices" : 3,
							"material" : 0
						}
					]
				}
			]
		})";

	auto gltf = glTFFromString(meshes);

	glTF::Meshes expectedMeshes{{
		"Cube", // name
		glTF::Mesh::Primitives{
			{
				{
					{"POSITION", 0},
					{"NORMAL", 1},
					{"TEXCOORD_0", 2},
				}, // attributes
				3, // indices
				0, // material
			}},
	}};

	EXPECT_EQ(expectedMeshes, gltf.meshes);
}
