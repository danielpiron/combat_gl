#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

class glTF
{
	class Asset
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
	glTF(const char *json_text)
	{
		auto j = nlohmann::json::parse(json_text);
		asset = Asset(j.at("asset").at("version").get<std::string>());
	}

public:
	Asset asset;
};

using namespace nlohmann;

TEST(glTFLoader, CanParseAssetVersion)
{
	const char *asset_version_only = R"({ "asset": { "version": "2.0" } })";
	glTF gltf(asset_version_only);

	EXPECT_EQ(std::string("2.0"), static_cast<std::string>(gltf.asset.version));
	EXPECT_EQ(2, gltf.asset.version.major());
	EXPECT_EQ(0, gltf.asset.version.minor());
}