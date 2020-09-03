#pragma once
#include <map>
#include <memory>
#include "hash.h"
#include "KGGeometry.h"
#include "KGShader.h"
#include "Texture.h"
namespace KG::Resource
{
	class ResourceContainer
	{
	private:
		std::map<KG::Utill::HashString, KG::Renderer::Shader> shaders;
		std::map<KG::Utill::HashString, KG::Renderer::Geometry> geometrys;
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
		KG::Renderer::Geometry* LoadGeometry( const KG::Utill::HashString& id );
		KG::Resource::Texture* LoadTexture( const KG::Utill::HashString& id );
		std::pair<size_t, KG::Utill::HashString> LoadMaterial( const KG::Utill::HashString& id );
		
		void Clear();
		static ResourceContainer* GetInstance();

		void Process(ID3D12GraphicsCommandList* cmdList);
	};
};