#pragma once
#include <set>
#include <map>
#include <type_traits>
#include <assert.h>
#include "hash.h"
#include "Debug.h"
#include "ISerializable.h"
#include "IComponent.h"

#define COMPONENT_CONTAINER_USE_VECTOR

namespace KG::Component
{
	struct ComponentContainer
	{
		std::vector<std::pair<KG::Utill::HashString, IComponent*>> container;
		//std::map<KG::Utill::HashString, IComponent*> container;

		inline void AddComponentWithID(const KG::Utill::HashString& componentID, IComponent* component)
		{
#ifdef COMPONENT_CONTAINER_USE_VECTOR
			this->container.emplace_back(std::make_pair(componentID, component));
#else
			auto [it, already] = this->container.insert(
				std::make_pair(componentID, component)
			);
			DebugAssertion(already, "이미 게임오브젝트에 포함되어있는 컴포넌트를 넣었습니다. ");
#endif
		}

		template <class Ty>
		inline void AddComponent(Ty* ptr)
		{
			AddComponentWithID(KG::Component::ComponentID<Ty>::id(), static_cast<IComponent*>(ptr));
		}

		inline IComponent* GetComponentWithID(const KG::Utill::HashString& componentID) const
		{
#ifdef COMPONENT_CONTAINER_USE_VECTOR
			auto it = std::find_if(this->container.begin(), this->container.end(), [&componentID](auto& p) {return p.first == componentID; });
			return (it != this->container.end()) ? it->second : nullptr;
#else
			auto it = this->container.find(componentID);
			return (it != this->container.end()) ? it->second : nullptr;
#endif
		}

		template <class Ty>
		inline Ty* GetComponent() const
		{
			return static_cast<Ty*>(
				GetComponentWithID(
					KG::Component::ComponentID<Ty>::id()
				)
				);
		}
	};
};