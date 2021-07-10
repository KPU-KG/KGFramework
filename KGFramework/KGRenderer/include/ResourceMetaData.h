#pragma once
#include <string>
namespace KG::Resource::Metadata
{
	struct ShaderSetData
	{
		std::string fileDir;
		int shaderGroup;
		bool enableCullBackface;
		bool enableDepthCliping;
		int materialParameterSize;
        UINT unitSizeX;
        UINT unitSizeY;
        UINT unitSizeZ;
	};


	struct GeometrySetData
	{
		std::string fileDir;
		bool isRawMesh = false;
	};

	enum Dimension
	{
		Unknown = 0,
		Buffer = 1,
		Texture1D = 2,
		Texture1DArray = 3,
		Texture2D = 4,
		Texture2DArray = 5,
		Texture2DMS = 6,
		Texture2DMSArray = 7,
		Texture3D = 8,
		TextureCube = 9,
		TextureCubeArray = 10
	};

	enum Format
	{
		DDS,
		PNG,
		JPG,
		BMP,
		GIF,
		TIF
	};

	struct TextureData
	{
		std::string fileDir;
		Format format;
		Dimension dimension;
	};
};