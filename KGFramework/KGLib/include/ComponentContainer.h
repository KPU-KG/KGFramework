#pragma once
#include <set>
#include <map>
#include <type_traits>
#include <assert.h>
#include "hash.h"
#include "Debug.h"
#include "IComponent.h"

namespace KG::Component
{
	/// @brief 
	class ComponentContainer
	{
		std::map<KG::Utill::HashString, IComponent*> container;
	public:
		template <class Ty>
		void AddComponent( Ty* ptr )
		{
			auto [it, already] = this->container.insert(
				std::make_pair( KG::Utill::HashString( ComponentID<Ty>::id() ), ptr )
			);
			DebugAssertion( already , ComponentID<Ty>::name() << L"는 이미 게임오브젝트에 포함되어있습니다. " );
		}

		template <class Ty>
		Ty* GetComponent() const
		{
			//return static_cast<Ty*>(this->container.at( KG::Utill::HashString( ComponentID<Ty>::id() ) ));
			auto it = this->container.find( KG::Utill::HashString( ComponentID<Ty>::id() ) );
			return (it != this->container.end()) ? static_cast<Ty*>(it->second) : nullptr;
		}

	};
};