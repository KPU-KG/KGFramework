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
	};


	struct GeometrySetData
	{
		std::string fileDir;
	};
};