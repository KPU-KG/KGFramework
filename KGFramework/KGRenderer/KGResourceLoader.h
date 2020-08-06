#pragma once
#include <d3d12.h>
#include <string>
#include "hash.h"
namespace KG::Renderer
{
	class IShader;
}
namespace KG::Resource
{
	enum class ResourceType
	{
		Geometry,
		Texture,
		ShdaerScript,
	};
	struct ResourceLoader
	{
		static ID3DBlob* CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile);
		static ID3DBlob* LoadShaderFromFile(const std::string& xmlFileDir, const KG::Utill::HashString& targetId);
	};
};