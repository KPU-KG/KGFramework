#pragma once
#include <string>
namespace KG::Resource::Metadata
{
	struct ShaderCodeData
	{
		bool isEnable = false;
		std::string fileDir;
		std::string entry;
		std::string type;
	};
	struct ShaderSetData
	{
		ShaderCodeData vertexShader;
		ShaderCodeData geometryShader;
		ShaderCodeData domainShader;
		ShaderCodeData hullShader;
		ShaderCodeData pixelShader;

		bool enableCullBackface;
		bool enableDepthCliping;
		std::string blendOpType;
		int renderPriority;
		int shaderType;
		int materialParameterSize;
	};


	struct GeometrySetData
	{
		std::string fileDir;
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