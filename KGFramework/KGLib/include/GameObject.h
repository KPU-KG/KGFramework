#pragma once
#include <string>
#include "IComponent.h"
#include "ComponentContainer.h"
namespace KG::Core
{
	using KG::Component::ComponentContainer;
	class GameObject
	{
		ComponentContainer components;
	public:
		std::string name;

		template <class Ty>
		auto GetComponent() { return this->components.GetComponent<Ty>(); }

		template <class Ty>
		auto AddComponent(Ty* cmp) 
		{
			cmp->OnCreate( this );
			return this->components.AddComponent<Ty>(cmp);
		}
	};
}