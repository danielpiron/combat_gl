#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <cstring>
#include <string>
#include <vector>

int base64CharacterValue(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c - 'A';
	}
	else if (c >= 'a' && c <= 'z')
	{
		return c - 'a' + 26;
	}
	else if (c >= '0' && c <= '9')
	{
		return c - '0' + 52;
	}
	else if (c == '+')
	{
		return 62;
	}
	else if (c == '/')
	{
		return 63;
	}
	return -1;
}

int decodeBase64Chunk(const char *base64Input, char *output)
{
	int base64Values[] = {-1, -1, -1, -1};

	for (size_t i = 0; i < 4; ++i)
	{
		base64Values[i] = base64CharacterValue(base64Input[i]);
	}

	int decodedChars = 1;
	output[0] = base64Values[0] << 2 | base64Values[1] >> 4;

	if (base64Values[2] != -1)
	{
		output[1] = (base64Values[1] & 0x0F) << 4 | base64Values[2] >> 2;
		decodedChars++;
	}
	if (base64Values[3] != -1)
	{
		output[2] = base64Values[2] << 6 | base64Values[3];
		decodedChars++;
	}

	return decodedChars;
}

int decodeBase64(const char *base64Input, char *output, size_t len)
{
	int totalBytesDecoded = 0;
	while (len > 0)
	{
		int bytesDecoded = decodeBase64Chunk(base64Input, output);
		output += bytesDecoded;
		base64Input += 4;
		totalBytesDecoded += bytesDecoded;
		len -= 4;
	}
	return totalBytesDecoded;
}

TEST(Base64, CanDecodeThreeCharacters)
{
	const char *expected = "Man";
	const char base64String[] = {'T', 'W', 'F', 'u'};

	char decodedBuffer[4];
	std::memset(decodedBuffer, 0, 4);

	EXPECT_EQ(3, decodeBase64(base64String, decodedBuffer, 4));
	EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeCharactersWithPadding)
{
	const char *expected = "Ma";
	const char base64String[] = {'T', 'W', 'E', '='};

	char decodedBuffer[4];
	std::memset(decodedBuffer, 0, 4);

	EXPECT_EQ(2, decodeBase64(base64String, decodedBuffer, 4));
	EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeSingleCharacterWithPadding)
{
	const char *expected = "M";
	const char base64String[] = {'T', 'Q', '=', '='};

	char decodedBuffer[4];
	std::memset(decodedBuffer, 0, 4);

	EXPECT_EQ(1, decodeBase64(base64String, decodedBuffer, 4));
	EXPECT_STREQ(expected, decodedBuffer);
}

TEST(Base64, CanDecodeArbitrarilyLongSequence)
{
	const char *expected = "light work.";
	const char *base64String = "bGlnaHQgd29yay4=";

	char decodedBuffer[12];
	std::memset(decodedBuffer, 0, 12);

	EXPECT_EQ(11, decodeBase64(base64String, decodedBuffer, 16));
	EXPECT_STREQ(expected, decodedBuffer);
}

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

public:
	Asset asset;
	std::vector<BufferView> bufferViews;
	std::vector<Buffer> buffers;
};

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

void from_json(const nlohmann::json &j, glTF &gltf)
{
	j.at("asset").get_to(gltf.asset);
	if (j.count("bufferViews"))
	{
		j.at("bufferViews").get_to(gltf.bufferViews);
	}
	if (j.count("buffers"))
	{
		j.at("buffers").get_to(gltf.buffers);
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
				"byteOffset": 0,
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