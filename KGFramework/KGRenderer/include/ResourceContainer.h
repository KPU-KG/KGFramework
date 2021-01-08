#pragma once
#include <map>
#include <memory>
#include "hash.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "KGModel.h"
#include "KGShader.h"
#include "Texture.h"
#include "GameObject.h"
#include "ObjectContainer.h"
namespace KG::Resource
{
	class ResourceContainer
	{
	private:
		std::map<KG::Utill::HashString, KG::Renderer::Shader> shaders;
		std::map<KG::Utill::HashString, KG::Resource::FrameModel> models;
		std::map<std::pair<KG::Utill::HashString, UINT>, KG::Renderer::Geometry> geometrys;
		std::map<KG::Utill::HashString, KG::Resource::Texture> textures;
		std::map<KG::Utill::HashString, std::pair<size_t, KG::Utill::HashString>> materials;

		static std::unique_ptr<ResourceContainer> instance;
	public:
		ResourceContainer();
		~ResourceContainer();

		ResourceContainer( const ResourceContainer& other ) = delete;
		ResourceContainer( ResourceContainer&& other ) = delete;
		ResourceContainer& operator=( const ResourceContainer& other ) = delete;
		ResourceContainer& operator=( ResourceContainer&& other ) = delete;

		KG::Renderer::Shader* LoadShader(const KG::Utill::HashString& id);
		KG::Resource::FrameModel* LoadModel( const KG::Utill::HashString& id );
		KG::Renderer::Geometry* LoadGeometry( const KG::Utill::HashString& id, UINT geometryIndex = 0 );
		KG::Renderer::Geometry* LoadRawModel( const KG::Utill::HashString& id );
		KG::Renderer::Geometry* CreateGeometry( const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh );
		KG::Resource::Texture* LoadTexture( const KG::Utill::HashString& id );
		KG::Resource::Texture* CreateTexture( const KG::Utill::HashString& id, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
		std::pair<size_t, KG::Utill::HashString> LoadMaterial( const KG::Utill::HashString& id );
		

		void ConvertNodeToObject( const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node, const MaterialMatch& materials );
		KG::Core::GameObject* CreateObjectFromModel( const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const MaterialMatch& materials );

		void Clear();
		static ResourceContainer* GetInstance();

		void Process(ID3D12GraphicsCommandList* cmdList);
	};
};