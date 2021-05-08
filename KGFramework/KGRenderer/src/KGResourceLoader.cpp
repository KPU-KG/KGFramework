#include "pch.h"
#include "tinyxml2.h"
#include "KGResourceLoader.h"
#include "hash.h"
#include "debug.h"
#include "KGShader.h"
#include "ResourceMetaData.h"
#include "ResourceContainer.h"

#include <array>
#include <string>
#include <string_view>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>

using namespace std::string_literals;

static int ReadShaderGroup( const std::string& attribute )
{
	if ( attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::Opaque))
		return KG::Renderer::ShaderGroup::Opaque;
	else if(attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::SkyBox))
		return KG::Renderer::ShaderGroup::SkyBox;
	else if(attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::Transparent))
		return KG::Renderer::ShaderGroup::Transparent;
	else if(attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::AmbientLight))
		return KG::Renderer::ShaderGroup::AmbientLight;
	else if(attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::MeshVolumeLight))
		return KG::Renderer::ShaderGroup::MeshVolumeLight;
	else if ( attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::DirectionalLight) )
		return KG::Renderer::ShaderGroup::DirectionalLight;
	else if ( attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::ParticleAdd) )
		return KG::Renderer::ShaderGroup::ParticleAdd;
	else if ( attribute == KG::Renderer::ConvertToMacroString(KG::Renderer::ShaderGroup::ParticleTransparent) )
		return KG::Renderer::ShaderGroup::ParticleTransparent;
	else return -1;
}

KG::Resource::Metadata::ShaderSetData KG::Resource::ResourceLoader::LoadShaderSetFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID )
{
	bool isDirty = false;
	KG::Resource::Metadata::ShaderSetData data;
	tinyxml2::XMLDocument doc;
	doc.LoadFile( xmlDir.c_str() );
	auto shaderSets = doc.FirstChildElement( "ShaderSet" )->FirstChildElement( "SurfaceShaderSet" );
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
			data.shaderGroup = ReadShaderGroup( shaderSets->Attribute( "group" ) );
			data.enableCullBackface = shaderSets->BoolAttribute( "enableBackfaceCulling" );
			data.enableDepthCliping = shaderSets->BoolAttribute( "enableDepthCliping" );
			data.materialParameterSize = shaderSets->IntAttribute( "materialParameterSize" );
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
			data.isRawMesh = shaderSets->BoolAttribute( "rawMesh" );
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


static KG::Resource::Metadata::Format GetTextureFormat( const std::string_view& format )
{
	if ( format == "DDS" ) return KG::Resource::Metadata::Format::DDS;

	DebugAssertion( false, L"Texture Format is Not Implement" );
}
static KG::Resource::Metadata::Dimension GetTextureDimenstion( const std::string_view& dimension )
{
	if ( dimension == "Texture2D" ) return KG::Resource::Metadata::Dimension::Texture2D;
	if ( dimension == "TextureCube" ) return KG::Resource::Metadata::Dimension::TextureCube;

	DebugAssertion( false, L"Texture Dimension is UnknownType" );
}

KG::Resource::Metadata::TextureData KG::Resource::ResourceLoader::LoadTextureFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID )
{
	bool isDirty = false;
	KG::Resource::Metadata::TextureData data;
	tinyxml2::XMLDocument doc;
	doc.LoadFile( xmlDir.c_str() );
	auto shaderSets = doc.FirstChildElement( "TextureSet" )->FirstChildElement( "Texture" );
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
			data.dimension = GetTextureDimenstion( shaderSets->Attribute( "dimension" ) );
			data.format = GetTextureFormat( shaderSets->Attribute( "format" ) );
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

namespace MaterialParser
{
	static bool Texture( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto id = xml->Attribute( "id" );
		auto hash = xml->Attribute( "hash_id" );
		unsigned hash_id = 0;
		if ( !hash )
		{
			hash_id = KG::Utill::HashString( id ).value;
			xml->SetAttribute( "hash_id", hash_id );
			isDirty = true;
		}
		else
		{
			hash_id = std::stoul( hash );
		}

		auto texture = KG::Resource::ResourceContainer::GetInstance()->LoadTexture( KG::Utill::HashString( hash_id ) );
		element.Set( offset, texture->index );
		offset += byte;
		return true;
	}

	static bool FLOAT1( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "x" );
		element.Set( offset, x );
		offset += byte;
		return true;
	}

	static bool FLOAT2( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "x" );
		auto y = xml->FloatAttribute( "y" );
		element.Set( offset, x );
		element.Set( offset + 4, y );
		offset += byte;
		return true;
	}

	static bool FLOAT3( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "x" );
		auto y = xml->FloatAttribute( "y" );
		auto z = xml->FloatAttribute( "z" );
		element.Set( offset, x );
		element.Set( offset + 4, y );
		element.Set( offset + 8, z );
		offset += byte;
		return true;
	}

	static bool FLOAT4( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "x" );
		auto y = xml->FloatAttribute( "y" );
		auto z = xml->FloatAttribute( "z" );
		auto w = xml->FloatAttribute( "w" );
		element.Set( offset, x );
		element.Set( offset + 4, y );
		element.Set( offset + 8, z );
		element.Set( offset + 12, w );
		offset += byte;
		return true;
	}

	static bool Color3( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "r" );
		auto y = xml->FloatAttribute( "g" );
		auto z = xml->FloatAttribute( "b" );
		element.Set( offset, x );
		element.Set( offset + 4, y );
		element.Set( offset + 8, z );
		offset += byte;
		return true;
	}

	static bool Color4( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto x = xml->FloatAttribute( "r" );
		auto y = xml->FloatAttribute( "g" );
		auto z = xml->FloatAttribute( "b" );
		auto w = xml->FloatAttribute( "a" );
		element.Set( offset, x );
		element.Set( offset + 4, y );
		element.Set( offset + 8, z );
		element.Set( offset + 12, w );
		offset += byte;
		return true;
	}

	static bool ColorCode( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		size_t byte = xml->IntAttribute( "byte" );
		auto code = xml->Attribute( "code" );
		int r, g, b;
		sscanf_s( code, "%02x%02x%02x", &r, &g, &b );
		float a = 1.0f;
		element.Set( offset, r / 255.0f );
		element.Set( offset + 4, g / 255.0f );
		element.Set( offset + 8, b / 255.0f );
		element.Set( offset + 12, a / 255.0f );
		offset += byte;
		return true;
	}

	static bool Padding( tinyxml2::XMLElement* xml, KG::Resource::DynamicElementInterface& element, int& offset, bool& isDirty )
	{
		if ( !(xml->Name() == "Padding"s) )
			return false;
		size_t byte = xml->IntAttribute( "byte" );
		offset += byte;
		return true;
	}

	static std::map<std::string_view, std::function<bool( tinyxml2::XMLElement*, KG::Resource::DynamicElementInterface&, int&, bool& )>> parsers =
	{
		std::make_pair( "Texture", Texture ),
		std::make_pair( "FLOAT1", FLOAT1 ),
		std::make_pair( "FLOAT2", FLOAT2 ),
		std::make_pair( "FLOAT3", FLOAT3 ),
		std::make_pair( "FLOAT4", FLOAT4 ),
		std::make_pair( "Color3", Color3 ),
		std::make_pair( "Color4", Color4 ),
		std::make_pair( "ColorCode", ColorCode ),
		std::make_pair( "Padding", Padding )
	};
};

std::pair<size_t, KG::Utill::HashString> KG::Resource::ResourceLoader::LoadMaterialFromFile( const std::string& xmlDir, const KG::Utill::HashString& targetID )
{
	bool isDirty = false;
	KG::Resource::Metadata::TextureData data;
	tinyxml2::XMLDocument doc;

	size_t index;
	KG::Utill::hashType shaderHashID;

	doc.LoadFile( xmlDir.c_str() );
	auto shaderSets = doc.FirstChildElement( "MaterialSet" )->FirstChildElement( "Material" );
	while ( shaderSets )
	{
		auto id = shaderSets->Attribute( "id" );
		auto hash = shaderSets->Attribute( "hash_id" );
		auto shaderId = shaderSets->Attribute( "shaderID" );
		auto shaderHash = shaderSets->Attribute( "shaderHashID" );
		unsigned hash_id = 0;
		unsigned shaderHash_id = 0;
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

		if ( !shaderHash )
		{
			shaderHash_id = KG::Utill::HashString( shaderId ).value;
			shaderSets->SetAttribute( "shaderHashID", shaderHash_id );
			isDirty = true;
		}
		else
		{
			shaderHash_id = std::stoul( shaderHash );
		}

		if ( hash_id == targetID.value )
		{
			shaderHashID = shaderHash_id;

			KG::Renderer::Shader* currentShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader( shaderHash_id );
			//메인 로직
			auto IDSTRING = KG::Utill::HashString( hash_id );
			if ( !currentShader->CheckMaterialLoaded( IDSTRING ) )
			{
				index = currentShader->RequestMaterialIndex( IDSTRING );
				auto elementInterface = currentShader->GetMaterialElement( IDSTRING );
				auto childs = shaderSets->FirstChildElement();
				int offset = 0;
				while ( childs )
				{
					DebugNormalMessage( L"메테리얼 읽는 중 " )
						MaterialParser::parsers.at( childs->Name() )(childs, elementInterface, offset, isDirty);
					childs = childs->NextSiblingElement();
				}
			}
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
	return std::make_pair( index, shaderHashID );
}
