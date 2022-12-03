#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

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
	struct BufferView
	{
		int buffer;
		int byteOffset;
		int byteLength;
		int target;

		bool operator==(const BufferView &rhs) const
		{
			return buffer == rhs.buffer && byteOffset == rhs.byteOffset &&
				   byteLength == rhs.byteLength && target == rhs.target;
		}
	};

public:
	Asset asset;
	std::vector<BufferView> bufferViews;
};

void from_json(const nlohmann::json &j, glTF::Asset::Version &v)
{
	v = glTF::Asset::Version(j.get<std::string>());
}

void from_json(const nlohmann::json &j, glTF::Asset &a)
{
	j.at("version").get_to(a.version);
}

void from_json(const nlohmann::json &j, glTF::BufferView &bv)
{
	j.at("buffer").get_to(bv.buffer);
	j.at("byteOffset").get_to(bv.byteOffset);
	j.at("byteLength").get_to(bv.byteLength);
	j.at("target").get_to(bv.target);
}

void from_json(const nlohmann::json &j, glTF &gltf)
{
	j.at("asset").get_to(gltf.asset);
	if (j.count("bufferViews"))
	{
		j.at("bufferViews").get_to(gltf.bufferViews);
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
			    "target": 34962
			  }
		    ]
		  })";
	auto gltf = glTFFromString(bufferViews);

	glTF::BufferView firstView{0, 0, 6, 34963};
	glTF::BufferView secondView{0, 8, 36, 34962};

	ASSERT_EQ(2, gltf.bufferViews.size());
	EXPECT_EQ(firstView, gltf.bufferViews[0]);
	EXPECT_EQ(secondView, gltf.bufferViews[1]);
}