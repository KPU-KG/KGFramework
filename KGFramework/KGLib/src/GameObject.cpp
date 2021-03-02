#include "GameObject.h"
#include "Transform.h"

KG::Core::GameObject* KG::Core::GameObject::InternalFindChildObject( const KG::Utill::HashString& id ) const
{
    if ( this->id == id )
    {
        return const_cast<KG::Core::GameObject*>(this);
    }
    else 
    {
        auto* ch = this->GetChild();
        auto* sib = this->GetSibling();

        if ( ch != nullptr )
        {
            auto* res = ch->InternalFindChildObject( id );
            if ( res != nullptr ) return res;
        }

        if ( sib != nullptr )
        {
            auto* res = sib->InternalFindChildObject( id );
            if ( res != nullptr ) return res;
        }
    }
    return nullptr;
}

void KG::Core::GameObject::InternalMatchBoneToObject( const std::vector<KG::Utill::HashString>& ids, std::vector<KG::Core::GameObject*>& bones ) const
{
    for ( size_t i = 0; i < ids.size(); i++ )
    {
        if ( ids[i] == this->id )
        {
            bones[i] = const_cast<KG::Core::GameObject*>(this);
        }
    }

    auto* ch = this->GetChild();
    auto* sib = this->GetSibling();
    
    if ( ch != nullptr ) ch->InternalMatchBoneToObject( ids, bones );
    
    if ( sib != nullptr ) sib->InternalMatchBoneToObject( ids, bones );
}

bool KG::Core::GameObject::IsDestroy() const
{
    return this->isDestroy;
}

void KG::Core::GameObject::Destroy()
{
    this->isDestroy = true;
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

KG::Core::GameObject* KG::Core::GameObject::FindChildObject( const KG::Utill::HashString& id ) const
{
    if ( this->id == id )
    {
        return const_cast<KG::Core::GameObject*>(this);
    }
    else
    {
        auto* ch = this->GetChild();

        if ( ch != nullptr )
        {
            auto* res = ch->InternalFindChildObject( id );
            if ( res != nullptr ) return res;
        }
    }
    return nullptr;
}

void KG::Core::GameObject::MatchBoneToObject( const std::vector<KG::Utill::HashString>& ids, std::vector<KG::Core::GameObject*>& bones ) const
{
    for ( size_t i = 0; i < ids.size(); i++ )
    {
        if ( ids[i] == this->id )
        {
            bones[i] = const_cast<KG::Core::GameObject*>(this);
        }
    }

    auto* ch = this->GetChild();
    if ( ch != nullptr ) ch->InternalMatchBoneToObject( ids, bones );
}
