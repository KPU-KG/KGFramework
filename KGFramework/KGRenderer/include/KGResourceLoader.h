#pragma once
#include <d3d12.h>
#include <string>
#include "hash.h"
#include "ResourceMetaData.h"

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
		static Metadata::ShaderSetData LoadShaderSetFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID );
		static Metadata::GeometrySetData LoadGeometrySetFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID );
	};
};