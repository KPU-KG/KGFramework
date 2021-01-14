#include "GameObject.h"
#include "Transform.h"

KG::Component::TransformComponent* KG::Core::GameObject::GetTransform() const
{
    return this->GetComponent<KG::Component::TransformComponent>();
}

KG::Core::GameObject* KG::Core::GameObject::FindChildObject( const KG::Utill::HashString& id ) const
{
    return nullptr;
}
