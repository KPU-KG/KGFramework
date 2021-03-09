#pragma once
#include <map>
#include <functional>
#include "hash.h"
#include "GameObject.h"
namespace KG::Component
{
	class ComponentProvider
	{
		std::map<KG::Utill::HashString, std::function<KG::Component::IComponent*(KG::Core::GameObject*)>> injectionFunctions;
	public:
		KG::Component::IComponent* AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject);
		void PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (KG::Core::GameObject*)> function);
	};
};
