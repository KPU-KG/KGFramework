#include "Scene.h"
#include <algorithm>

KG::Core::GameObject* KG::Core::Scene::CreateNewObject( const KG::Utill::HashString& id )
{
    auto it = std::find_if( this->objectPool.begin(), this->objectPool.end(), []( const KG::Core::GameObject& obj ) { return !obj.IsDestroy(); } );
    if ( it == this->activePool.end() )
    {

    }
    else 
    {
        return &(*it);
    }
}

void KG::Core::Scene::DeleteObject( const KG::Utill::HashString& id )
{
}

void KG::Core::Scene::FindObject( const KG::Utill::HashString& id )
{
}

void KG::Core::Scene::ClearObject()
{
}
