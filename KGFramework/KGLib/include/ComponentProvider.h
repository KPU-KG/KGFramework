#pragma once
#include <map>
#include <functional>
#include "hash.h"
#include "GameObject.h"
namespace KG::Component
{
	class ComponentProvider
	{
		std::map<KG::Utill::HashString, std::function<void(KG::Core::GameObject*)>> injectionFunctions;
	public:
		void AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject);
		void PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<void(KG::Core::GameObject*)> function);
	};
};
