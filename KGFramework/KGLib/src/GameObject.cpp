#include "GameObject.h"
#include "Transform.h"

KG::Component::TransformComponent* KG::Core::GameObject::GetTransform() const
{
    return this->GetComponent<KG::Component::TransformComponent>();
}
