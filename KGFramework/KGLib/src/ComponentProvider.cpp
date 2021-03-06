#include "ComponentProvider.h"
void KG::Component::ComponentProvider::AddComponentToObject(const KG::Utill::HashString& componentId, KG::Core::GameObject* targetObject)
{
	this->injectionFunctions[componentId](targetObject);
}

void KG::Component::ComponentProvider::PostInjectionFunction(const KG::Utill::HashString& componentId, std::function<void(KG::Core::GameObject*)> function)
{
	this->injectionFunctions.emplace(std::make_pair(componentId, function));
}