#pragma once
#include <map>
#include <functional>
#include "hash.h"
#include "GameObject.h"
namespace KG::Component
{
	class ComponentProvider
	{
	public:
		std::map<KG::Utill::HashString, std::function<KG::Component::IComponent* (KG::Core::GameObject*)>> injectionFunctions;
		std::map<KG::Utill::HashString, std::function<KG::Component::IComponent* ()>> getterFunctions;
		KG::Component::IComponent* AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject);
		KG::Component::IComponent* GetComponent(const KG::Utill::HashString& componentId);
		void PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (KG::Core::GameObject*)> function);
		void PostGetterFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (void)> function);
	};
};
