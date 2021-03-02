#pragma once
#include <string>
#include <vector>
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
		bool isDestroy = false;
		ComponentContainer components;

		KG::Core::GameObject* InternalFindChildObject( const KG::Utill::HashString& id ) const;
		void InternalMatchBoneToObject( const std::vector<KG::Utill::HashString>& ids, std::vector<KG::Core::GameObject*>& bones ) const;
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
		bool IsDestroy() const;
		void Destroy();

		KG::Component::TransformComponent* GetTransform() const;
		KG::Core::GameObject* GetChild() const;
		KG::Core::GameObject* GetSibling() const;
		KG::Core::GameObject* FindChildObject( const KG::Utill::HashString& id ) const;
		void MatchBoneToObject( const std::vector<KG::Utill::HashString>& ids, std::vector<KG::Core::GameObject*>& bones ) const;

	};
}