#pragma once
#include <set>
#include <map>
#include <type_traits>
#include <assert.h>
#include "hash.h"
#include "IComponent.h"
namespace KG::Component
{
	class ComponentContainer
	{
		std::map<KG::Utill::HashString, IComponent*> container;
	public:
		template <class Ty>
		void AddComponent(Ty* ptr)
		{
			auto [it, check] = this->container.insert(
				std::make_pair(KG::Utill::HashString(ComponentID<Ty>::id), ptr)
			);
			assert(check);
		}

		template <class Ty>
		Ty* GetComponent() const
		{
			auto it = this->container.find(KG::Utill::HashString(ComponentID<Ty>::id));
			return (it != this->container.end()) ? static_cast<Ty*>(it->second) : nullptr;
		}

	};
};