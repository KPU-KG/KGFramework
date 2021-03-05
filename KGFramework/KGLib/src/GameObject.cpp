#include "GameObject.h"
#include "Transform.h"
#include "Scene.h"

KG::Core::GameObject* KG::Core::GameObject::InternalFindChildObject( const KG::Utill::HashString& tag ) const
{
    if ( this->tag == tag )
    {
        return const_cast<KG::Core::GameObject*>(this);
    }
    else 
    {
        auto* ch = this->GetChild();
        auto* sib = this->GetSibling();

        if ( ch != nullptr )
        {
            auto* res = ch->InternalFindChildObject( tag );
            if ( res != nullptr ) return res;
        }

        if ( sib != nullptr )
        {
            auto* res = sib->InternalFindChildObject( tag );
            if ( res != nullptr ) return res;
        }
    }
    return nullptr;
}

void KG::Core::GameObject::InternalMatchBoneToObject( const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones ) const
{
    for ( size_t i = 0; i < tags.size(); i++ )
    {
        if ( tags[i] == this->tag )
        {
            bones[i] = const_cast<KG::Core::GameObject*>(this);
        }
    }

    auto* ch = this->GetChild();
    auto* sib = this->GetSibling();
    
    if ( ch != nullptr ) ch->InternalMatchBoneToObject( tags, bones );
    
    if ( sib != nullptr ) sib->InternalMatchBoneToObject( tags, bones );
}

bool KG::Core::GameObject::IsDestroy() const
{
    return this->isDestroy;
}

void KG::Core::GameObject::Destroy()
{
    this->isDestroy = true;
}

void KG::Core::GameObject::SetOwnerScene( KG::Core::Scene* ownerScene )
{
    this->ownerScene = ownerScene;
}

void KG::Core::GameObject::SetInstanceID( UINT32 instanceID )
{
    this->instanceID = instanceID;
}

KG::Component::TransformComponent* KG::Core::GameObject::GetTransform() const
{
    return this->GetComponent<KG::Component::TransformComponent>();
}

inline KG::Core::GameObject* KG::Core::GameObject::GetChild() const
{
    return this->GetTransform()->hasChild() ? this->GetTransform()->GetChild()->GetGameObject() : nullptr;
}

inline KG::Core::GameObject* KG::Core::GameObject::GetSibling() const
{
    return this->GetTransform()->hasSibiling() ? this->GetTransform()->GetNextsibiling()->GetGameObject() : nullptr;
}

KG::Core::GameObject* KG::Core::GameObject::FindChildObject( const KG::Utill::HashString& tag ) const
{
    if ( this->tag == tag )
    {
        return const_cast<KG::Core::GameObject*>(this);
    }
    else
    {
        auto* ch = this->GetChild();

        if ( ch != nullptr )
        {
            auto* res = ch->InternalFindChildObject( tag );
            if ( res != nullptr ) return res;
        }
    }
    return nullptr;
}

KG::Core::Scene* KG::Core::GameObject::GetScene() const
{
    return this->ownerScene;
}

UINT32 KG::Core::GameObject::GetInstanceID() const
{
    return this->instanceID;
}

void KG::Core::GameObject::MatchBoneToObject( const std::vector<KG::Utill::HashString>& tags, std::vector<KG::Core::GameObject*>& bones ) const
{
    for ( size_t i = 0; i < tags.size(); i++ )
    {
        if ( tags[i] == this->tag )
        {
            bones[i] = const_cast<KG::Core::GameObject*>(this);
        }
    }

    auto* ch = this->GetChild();
    if ( ch != nullptr ) ch->InternalMatchBoneToObject( tags, bones );
}
