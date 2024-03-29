#include <gtest/gtest.h>

#include <util/gltf.h>

std::ostream &operator<<(std::ostream &os, const glTF::Material::PbrMetallicRoughness &pbr)
{
	os << "{ baseColorFactor: "
	   << "[ " << pbr.baseColorFactor[0]
	   << ", " << pbr.baseColorFactor[1]
	   << ", " << pbr.baseColorFactor[2]
	   << " ]"
	   << ", metallicFactor: " << pbr.metallicFactor
	   << ", roughnessFactor: " << pbr.roughnessFactor;
	return os;
}

std::ostream &operator<<(std::ostream &os, const glTF::Material &m)
{
	os << "{ alphaMode: " << m.alphaMode
	   << ", doubleSided: " << m.doubleSided
	   << ", emissiveFactor [ " << m.emissiveFactor[0]
	   << ", " << m.emissiveFactor[1]
	   << ", " << m.emissiveFactor[2]
	   << " ]"
	   << ", name: \"" << m.name << "\""
	   << ", pbrMetallicRoughness: \"" << m.pbrMetallicRoughness << "\""
	   << " }";
	return os;
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
	EXPECT_EQ(1, gltf.accessors[0].componentCount());
	EXPECT_EQ(3, gltf.accessors[1].componentCount());
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
				},										// attributes
				3,										// indices
				0,										// material
				glTF::Mesh::Primitive::Mode::TRIANGLES, // mode
			}},
	}};

	EXPECT_EQ(expectedMeshes, gltf.meshes);
}

TEST(glTFLoader, CanParseMaterials)
{
	const char *materials = R"(
{
  "materials" : [
    {
		"pbrMetallicRoughness": {
			"baseColorFactor": [ 1.000, 0.766, 0.336, 1.0 ],
			"metallicFactor": 0.5,
			"roughnessFactor": 0.1
		}
	},
	{
		"doubleSided" : true,
		"name" : "Treads",
		"pbrMetallicRoughness" : {
			"baseColorFactor" : [
				0.017387472093105316,
				0.012161456979811192,
				0.014963649213314056,
				1
			],
			"metallicFactor" : 0,
			"roughnessFactor" : 0.5
		}
	}

  ],
  "asset" : {
    "version" : "2.0"
  }
}
)";

	std::vector<glTF::Material>
		expectedMaterials{
			{"OPAQUE",	// alphaMode
			 false,		// doubleSided
			 {0, 0, 0}, // emissiveFactor
			 "",		// name
			 {
				 {1.000, 0.766, 0.336, 1.0}, // baseColorFactor
				 0.5,						 // metallicFactor
				 0.1,						 // roughnessFactor
			 }},
			{"OPAQUE",	// alphaMode
			 true,		// doubleSided
			 {0, 0, 0}, // emissiveFactor
			 "Treads",	// name
			 {
				 {0.017387472093105316f,
				  0.012161456979811192f,
				  0.014963649213314056f,
				  1.0f}, // baseColorFactor
				 0,		 // metallicFactor
				 0.5,	 // roughnessFactor
			 }},

		};

	auto gltf = glTFFromString(materials);
	EXPECT_EQ(expectedMaterials, gltf.materials);
}