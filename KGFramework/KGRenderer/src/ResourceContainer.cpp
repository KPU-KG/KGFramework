#include "pch.h"
#include <map>
#include <mutex>
#include <algorithm>
#include <stack>
#include "Debug.h"
#include "ResourceContainer.h"
#include "KGShader.h"
#include "KGGeometry.h"
#include "KGResourceLoader.h"
#include "KGDXRenderer.h"
#include "KGModel.h"
#include "Transform.h"

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

KG::Resource::FrameModel* KG::Resource::ResourceContainer::LoadModel( const KG::Utill::HashString& id )
{
	if ( this->models.count( id ) )
	{
		return &this->models.at( id );
	}
	else
	{
		auto metaData = ResourceLoader::LoadGeometrySetFromFile( "Resource/GeometrySet.xml", id );
		return &this->models.emplace( id, metaData ).first->second;
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadGeometry( const KG::Utill::HashString& id, UINT geometryIndex )
{
	auto handle = std::make_pair( id, geometryIndex );
	if ( this->geometrys.count( handle ) )
	{
		return &this->geometrys.at( handle );
	}
	else
	{
		auto metaData = ResourceLoader::LoadGeometrySetFromFile( "Resource/GeometrySet.xml", id );
		if ( metaData.isRawMesh )
		{
			return this->LoadRawModel( id );
		}
		else
		{
			DebugErrorMessage( "Try Load Invalide model's Geometry" );
			return nullptr;
		}
	}
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::LoadRawModel( const KG::Utill::HashString& id )
{
	auto* model = this->LoadModel( id );
	return &this->geometrys.emplace( std::make_pair( id, 0 ), model->data.meshs[0] ).first->second;
}

KG::Renderer::Geometry* KG::Resource::ResourceContainer::CreateGeometry( const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh )
{
	auto handle = std::make_pair( id, geometryIndex );
	if ( this->geometrys.count( handle ) )
	{
		return &this->geometrys.at( handle );
		DebugErrorMessage( "Try Load Already Loaded model's Geometry" )
	}
	else
	{
		this->geometrys.emplace( std::make_pair( handle, mesh ) );
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

KG::Resource::Texture* KG::Resource::ResourceContainer::CreateTexture( const KG::Utill::HashString& id, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc )
{
	if ( this->textures.count( id ) )
	{
		DebugErrorMessage( id.value << L"는 이미 존재하는 텍스처입니다." );
		return &this->textures.at( id );
	}
	else
	{
		return &this->textures.emplace( id, KG::Resource::Texture( resource, srvDesc ) ).first->second;
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

void KG::Resource::ResourceContainer::ConvertNodeToObject( const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node )
{
	if ( !node->meshs.empty() )
	{
		auto* renderer = KG::Renderer::KGDXRenderer::GetInstance();
		auto index = node->meshs[0];
		object->AddComponent( renderer->GetNewGeomteryComponent( id, index ) );
	}
}

KG::Core::GameObject* KG::Resource::ResourceContainer::CreateObjectFromModel( const KG::Utill::HashString& id, KG::Core::ObjectContainer& container )
{
	auto& frame = this->models[id].data;
	std::stack<std::pair<KG::Core::GameObject*, KG::Utill::ModelNode*>> modelStack;
	auto* rootObject = container.CreateNewObject();
	auto* rootModelNode = frame.root;

	for ( size_t i = 0; i < frame.meshs.size(); i++ )
	{
		this->geometrys.emplace( std::make_pair( id, i ), frame.meshs[i] );
	}

	modelStack.push( std::make_pair( rootObject, rootModelNode ) );
	while ( modelStack.empty() )
	{
		auto* object = modelStack.top().first;
		auto* node = modelStack.top().second;
		if ( node->child != nullptr && !object->GetComponent<KG::Component::TransformComponent>()->hasChild() )
		{
			auto* childObject = container.CreateNewObject();
			auto* childNode = node->child;
			this->ConvertNodeToObject( id, childObject, childNode );
			modelStack.push( std::make_pair( childObject, childNode ) );
		}
		if ( node->sibling != nullptr && !object->GetComponent<KG::Component::TransformComponent>()->hasSibiling() )
		{
			auto* siblingObject = container.CreateNewObject();
			auto* siblingNode = node->sibling;
			this->ConvertNodeToObject( id, siblingObject, siblingNode );
			modelStack.push( std::make_pair( siblingObject, siblingNode ) );
		}
		modelStack.pop();
	}
	return rootObject;
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
