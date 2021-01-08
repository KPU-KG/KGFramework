#include "ObjectContainer.h"
using namespace KG::Core;

void KG::Core::ObjectContainer::PostTransformSystem( KG::System::TransformSystem* system )
{
    this->transformSystem = system;
}

GameObject* KG::Core::ObjectContainer::CreateNewObject()
{
    auto* obj = &this->pool.emplace_back();
    obj->AddComponent( this->transformSystem->GetNewComponent() );
    return obj;
}

GameObject* KG::Core::ObjectContainer::GetGameObject( const std::string& name ) const
{
    auto it = this->searchPool.find( name );
    return it != this->searchPool.end() ? it->second : nullptr;
}

void KG::Core::ObjectContainer::DeleteObject()
{
}
