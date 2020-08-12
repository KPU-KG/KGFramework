#pragma once
#include <string>
namespace KG::Resource::Metadata
{
	enum ShaderType
	{
		Opaque = 0,
		Transparent = 1
	};
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
		int renderPriority;
		ShaderType shaderType;
	};


	struct GeometrySetData
	{
		std::string fileDir;
	};
};