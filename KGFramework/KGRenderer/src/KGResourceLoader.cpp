#include "pch.h"
#include "tinyxml2.h"
#include "KGResourceLoader.h"
#include "hash.h"
#include "debug.h"
#include "ResourceMetaData.h"

#include <array>
#include <string>
#include <string_view>
#include <fstream>
#include <iterator>
#include <algorithm>

enum ShaderTarget
{
	VS = 0,
	DS = 1,
	HS = 3,
	PS = 4,
	GS = 2,
};
using namespace std::string_literals;
constexpr std::string_view shaderTargetElementName[] = {
	"VertexShader",
	"DomainShader",
	"GeometryShader",
	"HullShader",
	"PixelShader",
};

static KG::Resource::Metadata::ShaderType GetShaderType( const std::string_view& type )
{
	if ( type == "opaque" ) return KG::Resource::Metadata::ShaderType::Opaque;
	if ( type == "transparent" ) return KG::Resource::Metadata::ShaderType::Transparent;
}

static KG::Resource::Metadata::ShaderCodeData LoadShaderCodes( const tinyxml2::XMLElement* element )
{
	using KG::Resource::Metadata::ShaderCodeData;
	ShaderCodeData data;
	data.entry = element->Attribute( "entry" );
	data.fileDir = element->Attribute( "fileDir" );
	data.type = element->Attribute( "type" );
	data.isEnable = true;
	return data;
}

KG::Resource::Metadata::ShaderSetData KG::Resource::ResourceLoader::LoadShaderSetFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID )
{
	bool isDirty = false;
	KG::Resource::Metadata::ShaderSetData data;
	tinyxml2::XMLDocument doc;
	doc.LoadFile( xmlDir.c_str() );
	auto shaderSets = doc.FirstChildElement( "ShaderSet" )->FirstChildElement( "GraphicShaderSet" );
	while ( shaderSets )
	{
		auto id = shaderSets->Attribute( "id" );
		auto hash = shaderSets->Attribute( "hash_id" );
		unsigned hash_id = 0;
		if ( !hash )
		{
			hash_id = KG::Utill::HashString( id ).value;
			shaderSets->SetAttribute( "hash_id", hash_id );
			isDirty = true;
		}
		else
		{
			hash_id = std::stoul( hash );
		}
		if ( hash_id == targetID.value )
		{
			data.shaderType = GetShaderType( shaderSets->Attribute( "shaderType" ) );
			data.renderPriority = std::atoi( shaderSets->Attribute( "renderPriority" ) );
			data.enableCullBackface = std::atoi( shaderSets->Attribute( "enableBackfaceCulling" ) );

			auto vsElement = shaderSets->FirstChildElement( shaderTargetElementName[VS].data() );
			data.vertexShader = LoadShaderCodes( vsElement );

			auto psElement = shaderSets->FirstChildElement( shaderTargetElementName[PS].data() );
			data.pixelShader = LoadShaderCodes( psElement );

			auto hsElement = shaderSets->FirstChildElement( shaderTargetElementName[HS].data() );
			if ( hsElement )
				data.hullShader = LoadShaderCodes( hsElement );

			auto dsElement = shaderSets->FirstChildElement( shaderTargetElementName[DS].data() );
			if ( dsElement )
				data.domainShader = LoadShaderCodes( dsElement );

			auto gsElement = shaderSets->FirstChildElement( shaderTargetElementName[GS].data() );
			if ( gsElement )
				data.geometryShader = LoadShaderCodes( gsElement );
			
			break;
		}
		else
		{
			shaderSets = shaderSets->NextSiblingElement();
		}
	}
	if ( isDirty )
	{
		doc.SaveFile( xmlDir.c_str() );
	}
	return data;
}

KG::Resource::Metadata::GeometrySetData KG::Resource::ResourceLoader::LoadGeometrySetFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID )
{
	bool isDirty = false;
	KG::Resource::Metadata::GeometrySetData data;
	tinyxml2::XMLDocument doc;
	doc.LoadFile( xmlDir.c_str() );
	auto shaderSets = doc.FirstChildElement( "GeometrySet" )->FirstChildElement( "Geometry" );
	while ( shaderSets )
	{
		auto id = shaderSets->Attribute( "id" );
		auto hash = shaderSets->Attribute( "hash_id" );
		unsigned hash_id = 0;
		if ( !hash )
		{
			hash_id = KG::Utill::HashString( id ).value;
			shaderSets->SetAttribute( "hash_id", hash_id );
			isDirty = true;
		}
		else
		{
			hash_id = std::stoul( hash );
		}
		if ( hash_id == targetID.value )
		{
			data.fileDir = shaderSets->Attribute( "fileDir" );
			break;
		}
		else
		{
			shaderSets = shaderSets->NextSiblingElement();
		}
	}
	if ( isDirty )
	{
		doc.SaveFile( xmlDir.c_str() );
	}
	return data;
}
