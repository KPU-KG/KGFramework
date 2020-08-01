#pragma once
#include <map>
#include <memory>
#include "hash.h"
#include "KGGeometry.h"
#include "KGShader.h"
namespace KG::Renderer
{
	class Geometry;
	class IShader;
	class ResourceContainer
	{
		std::map<KG::Utill::_ID, std::unique_ptr<Geometry>> geometry;
		std::map<KG::Utill::_ID, std::unique_ptr<IShader>> shader;
	public:
		void AddResource(KG::Utill::_ID id, Geometry* ptr);
		void AddResource(KG::Utill::_ID id, IShader* ptr);
		void AddResource(KG::Utill::_ID id, std::unique_ptr<Geometry>&& ptr);
		void AddResource(KG::Utill::_ID id, std::unique_ptr<IShader>&& ptr);
		template <class Ty ,typename... Type>
		void EmplaceResource(KG::Utill::_ID id, Type&&... parameters...)
		{
			this->AddResource(id, std::make_unique<Ty>(std::forward<Type>(parameters)...));
		}

		IShader* GetShader(KG::Utill::_ID id);
		Geometry* GetGeometry(KG::Utill::_ID id);
	};
}