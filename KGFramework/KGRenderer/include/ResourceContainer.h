#pragma once
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include "hash.h"
#include "KGDXRenderer.h"
#include "KGGeometry.h"
#include "KGModel.h"
#include "KGShader.h"
#include "Texture.h"
#include "GameObject.h"
#include "ObjectContainer.h"
#include "BoneDataBufferManager.h"
namespace KG::Resource
{
	class ResourceContainer
	{
	private:
		std::map<KG::Utill::HashString, KG::Renderer::Shader> shaders;
		std::map<KG::Utill::HashString, KG::Resource::FrameModel> models;
		std::map<std::pair<KG::Utill::HashString, UINT>, KG::Renderer::Geometry> geometrys;
		std::map<std::pair<D3D12_PRIMITIVE_TOPOLOGY, int>, KG::Renderer::Geometry> fakeGeometrys;
		std::map<KG::Utill::HashString, KG::Resource::Texture> textures;
		std::map<KG::Utill::HashString, std::pair<size_t, KG::Utill::HashString>> materials;
		std::map<std::pair<KG::Utill::HashString, UINT>, KG::Utill::AnimationSet> animations;
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
		void PreLoadModels(const std::vector<KG::Utill::HashString>& vectors);
		KG::Renderer::Geometry* LoadGeometry( const KG::Utill::HashString& id, UINT geometryIndex = 0 );
		KG::Renderer::Geometry* LoadRawModel( const KG::Utill::HashString& id );
		KG::Renderer::Geometry* CreateGeometry( const KG::Utill::HashString& id, UINT geometryIndex, KG::Utill::MeshData& mesh );
		KG::Renderer::Geometry* CreateFakeGeometry( D3D12_PRIMITIVE_TOPOLOGY topology, int vertexCount );
		KG::Resource::Texture* LoadTexture( const KG::Utill::HashString& id );
		KG::Resource::Texture* CreateTexture( const KG::Utill::HashString& id, ID3D12Resource* resource, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc);
		KG::Utill::AnimationSet* LoadAnimation( const KG::Utill::HashString& id, UINT animationIndex );
		std::pair<size_t, KG::Utill::HashString> LoadMaterial( const KG::Utill::HashString& id );

		//void AddAnimation( const KG::Utill::HashString& id, UINT animationIndex, const KG::Utill::AnimationSet& animation );
		//void AddAnimation( const KG::Utill::HashString& id, UINT animationIndex, KG::Utill::AnimationSet&& animation );

		std::vector<std::function<void()>> postModelLoadFunctions;
		void ConvertNodeToObject( const KG::Utill::HashString& id, KG::Core::GameObject* object, KG::Utill::ModelNode* node, const MaterialMatch& materials, KG::Core::GameObject* rootObject );
		KG::Core::GameObject* CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::ObjectContainer& container, const MaterialMatch& materials);
		KG::Core::GameObject* CreateObjectFromModel(const KG::Utill::HashString& id, KG::Core::Scene& scene, const MaterialMatch& materials);

		void Clear();
		static ResourceContainer* GetInstance();

		void Process(ID3D12GraphicsCommandList* cmdList);
	};
};