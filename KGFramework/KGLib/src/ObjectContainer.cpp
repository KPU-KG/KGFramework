#include "ObjectContainer.h"
using namespace KG::Core;

GameObject* KG::Core::ObjectContainer::CreateNewObject()
{
    return &this->pool.emplace_back();
}

GameObject* KG::Core::ObjectContainer::GetGameObject( const std::string& name ) const
{
    auto it = this->searchPool.find( name );
    return it != this->searchPool.end() ? it->second : nullptr;
}

void KG::Core::ObjectContainer::DeleteObject()
{
}
