#pragma once
#include <map>
#include <memory>
#include "hash.h"
#include "KGGeometry.h"
#include "KGShader.h"
namespace KG::Resource
{
	class ResourceContainer
	{
	private:
		std::map<KG::Utill::HashString, KG::Renderer::Shader> shaders;
		std::map<KG::Utill::HashString, KG::Renderer::Geometry> geometrys;

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
		void Clear();
		static ResourceContainer* GetInstance();
	};
};