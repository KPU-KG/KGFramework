#pragma once
#include <string>
#include "IComponent.h"
#include "ComponentContainer.h"
#include <vector>
#include <map>

class aiAnimation;

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

		// 애니메이션 컴포넌트에 들어갈 내용들
		std::vector<aiAnimation*> animations;
		std::vector<std::pair<std::string, DirectX::XMFLOAT4X4>> GetAnimationMatrix(double animationTime, int animationType);
		/// ///////////////////////////////////////
		/// ///////////////////////////////////////

		template <class Ty>
		auto GetComponent() const { return this->components.GetComponent<Ty>(); }

		template <class Ty>
		auto AddComponent(Ty* cmp) 
		{
			cmp->Create( this );
			return this->components.AddComponent<Ty>(cmp);
		}

		KG::Component::TransformComponent* GetTransform() const;
		KG::Core::GameObject* FindChildObject( const KG::Utill::HashString& id ) const;

	};
}