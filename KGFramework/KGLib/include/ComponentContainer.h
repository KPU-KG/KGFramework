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

		template <class Ty>
		void AddComponent(Ty* ptr)
		{
			auto [it, check] = this->container.insert(
				std::make_pair(ComponentID<std::decay_t<Ty>>::id, ptr)
			);
			assert(check);
		}

		template <class Ty>
		void GetComponent() const
		{
			return this->container.at(ComponentID<std::decay_t<Ty>::id>);
		}
	};
};