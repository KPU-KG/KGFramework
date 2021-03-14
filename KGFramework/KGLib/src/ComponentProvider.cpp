#include "ComponentProvider.h"
KG::Component::IComponent* KG::Component::ComponentProvider::AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject)
{
	return this->injectionFunctions[componentId](targetObject);
}

KG::Component::IComponent* KG::Component::ComponentProvider::GetComponent(const KG::Utill::HashString& componentId)
{
	return this->getterFunctions[componentId]();
}

void KG::Component::ComponentProvider::PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (KG::Core::GameObject*)> function)
{
	this->injectionFunctions.emplace(std::make_pair(componentId, function));
}

void KG::Component::ComponentProvider::PostGetterFunction(const KG::Utill::HashString& componentId, std::function<KG::Component::IComponent* (void)> function)
{
	this->getterFunctions.emplace(std::make_pair(componentId, function));
}
