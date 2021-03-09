#include "ComponentProvider.h"
KG::Component::IComponent* KG::Component::ComponentProvider::AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject)
{
	return this->injectionFunctions[componentId](targetObject);
}

void KG::Component::ComponentProvider::PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (KG::Core::GameObject*)> function)
{
	this->injectionFunctions.emplace(std::make_pair(componentId, function));
}