#include "pch.h"
#include "TransformSystem.h"

void KG::System::TransformSystem::Initialize()
{
	//this->pool.Reserve(100);
}

void KG::System::TransformSystem::OnGetNewComponent( TransformComponent* tran )
{
	tran->SetPosition( 0, 0, 0 );
	tran->SetRotation( XMQuaternionIdentity() );
	tran->SetScale( 1, 1, 1 );
}

void KG::System::TransformSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::TransformSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::TransformSystem::OnPreRender()
{
}
