#include "pch.h"
#include <map>
#include <mutex>
#include "Debug.h"
#include "ResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "KGResourceLoader.h"

std::unique_ptr<KG::Resource::ResourceContainer> KG::Resource::ResourceContainer::instance = nullptr;

KG::Resource::ResourceContainer::ResourceContainer()
{
	DebugNormalMessage( TOSTRING( ResourceContainer ) << " : Initialize" );
}

KG::Resource::ResourceContainer::~ResourceContainer()
{
	DebugNormalMessage( TOSTRING( ResourceContainer ) << " : Removed" );
}

KG::Renderer::Shader* KG::Resource::ResourceContainer::LoadShader( const KG::Utill::HashString& id )
{
	if ( this->shaders.count( id ) )
	{
		return &this->shaders.at( id );
	}
	else 
	{
		auto metaData = ResourceLoader::LoadShaderSetFromFile( "Resource/ShaderCode.xml", id );
		return &this->shaders.emplace( id, metaData ).first->second;
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadGeometry( const KG::Utill::HashString& id )
{
	if ( this->geometrys.count( id ) )
	{
		return &this->geometrys.at( id );
	}
	else
	{
		auto metaData = ResourceLoader::LoadGeometrySetFromFile( "Resource/GeometrySet.xml", id );
		return &this->geometrys.emplace( id, metaData ).first->second;
	}
}

KG::Resource::Texture* KG::Resource::ResourceContainer::LoadTexture( const KG::Utill::HashString& id )
{
	if ( this->textures.count( id ) )
	{
		return &this->textures.at( id );
	}
	else
	{
		auto metaData = ResourceLoader::LoadTextureFromFile( "Resource/TextureSet.xml", id );
		return &this->textures.emplace( id, metaData ).first->second;
	}
}

std::pair<size_t, KG::Utill::HashString> KG::Resource::ResourceContainer::LoadMaterial( const KG::Utill::HashString& id )
{
	if ( this->materials.count( id ) )
	{
		return this->materials.at( id );
	}
	else
	{
		auto result = KG::Resource::ResourceLoader::LoadMaterialFromFile( "Resource/MaterialSet.xml", id );
		return this->materials.emplace( id, result ).first->second;
	}
}

void KG::Resource::ResourceContainer::Clear()
{
	this->shaders.clear();
	this->geometrys.clear();
	this->textures.clear();
	DebugNormalMessage( TOSTRING( ResourceContainer ) << "All Resource Cleared" );
}

KG::Resource::ResourceContainer* KG::Resource::ResourceContainer::GetInstance()
{
	//static std::once_flag onceFlags;
	//std::call_once( onceFlags, []() {ResourceContainer::instance = std::make_unique<ResourceContainer>(); } );
	if ( ResourceContainer::instance.get() == nullptr )
	{
		ResourceContainer::instance = std::make_unique<ResourceContainer>();
	}
	return ResourceContainer::instance.get();
}

void KG::Resource::ResourceContainer::Process( ID3D12GraphicsCommandList* cmdList )
{
	for ( auto& [key, value] : this->textures )
	{
		value.Process( cmdList );
	}
}
