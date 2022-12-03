#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

class glTF
{
	class Asset
	{
	public:
		Asset() : _majorVersion(-1), _minorVersion(-1) {}

		Asset(const std::string &version)
		{
			auto periodIndex = version.find('.');
			_majorVersion = std::stoi(version.substr(0, periodIndex));
			_minorVersion = std::stoi(version.substr(periodIndex + 1));
		}
		int majorVersion() const { return _majorVersion; }
		int minorVersion() const { return _minorVersion; }

	private:
		int _majorVersion;
		int _minorVersion;
	};

public:
	glTF(const char *json_text)
	{
		auto j = nlohmann::json::parse(json_text);
		_asset = Asset(j["asset"]["version"].get<std::string>());
	}

public:
	const Asset &asset() const { return _asset; }

private:
	Asset _asset;
};

using namespace nlohmann;

TEST(glTFLoader, CanParseAssetVersion)
{
	const char *asset_version_only = R"({ "asset": { "version": "2.0" } })";
	glTF gltf(asset_version_only);

	EXPECT_EQ(2, gltf.asset().majorVersion());
	EXPECT_EQ(0, gltf.asset().minorVersion());
}