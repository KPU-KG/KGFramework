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
	auto metaData = ResourceLoader::LoadShaderSetFromFile( "Resource/ShaderCode.xml", id );
	return &this->shaders.emplace( id, metaData ).first->second;
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadGeometry( const KG::Utill::HashString& id )
{
	auto metaData = ResourceLoader::LoadGeometrySetFromFile( "Resource/GeometrySet.xml", id );
	auto* geo = &this->geometrys.emplace( id, metaData ).first->second;
	return geo;
}

void KG::Resource::ResourceContainer::Clear()
{
	this->shaders.clear();
	this->geometrys.clear();
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
