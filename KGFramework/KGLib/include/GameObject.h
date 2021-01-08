#pragma once
#include <string>
#include "IComponent.h"
#include "ComponentContainer.h"
namespace KG::Component
{
	class TransformComponent;
}
namespace KG::Core
{
	using KG::Component::ComponentContainer;
	class GameObject
	{
		ComponentContainer components;
	public:
		KG::Utill::HashString id = KG::Utill::HashString(0);
		std::string name;

		template <class Ty>
		auto GetComponent() const { return this->components.GetComponent<Ty>(); }

		template <class Ty>
		auto AddComponent(Ty* cmp) 
		{
			cmp->Create( this );
			return this->components.AddComponent<Ty>(cmp);
		}

		KG::Component::TransformComponent* GetTransform() const;
	};
}