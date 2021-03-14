#pragma once
#include <set>
#include <map>
#include <type_traits>
#include <assert.h>
#include "hash.h"
#include "Debug.h"
#include "ISerializable.h"
#include "IComponent.h"

namespace KG::Component
{
	struct ComponentContainer
	{
		std::map<KG::Utill::HashString, IComponent*> container;

		inline void AddComponentWithID(const KG::Utill::HashString& componentID, IComponent* component)
		{
			auto [it, already] = this->container.insert(
				std::make_pair(componentID, component)
			);
			DebugAssertion(already, "이미 게임오브젝트에 포함되어있는 컴포넌트를 넣었습니다. ");
		}

		template <class Ty>
		inline void AddComponent(Ty* ptr)
		{
			AddComponentWithID(KG::Component::ComponentID<Ty>::id(), static_cast<IComponent*>(ptr));
		}

		inline IComponent* GetComponentWithID(const KG::Utill::HashString& componentID) const
		{
			auto it = this->container.find(componentID);
			return (it != this->container.end()) ? it->second : nullptr;
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