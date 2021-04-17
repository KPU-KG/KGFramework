#include "PhysicsSystem.h"

void KG::System::DynamicRigidSystem::OnUpdate(float elapsedTime)
{
	for (auto& com : this->pool)
		com.Update(elapsedTime);
}

void KG::System::DynamicRigidSystem::OnPostUpdate(float elapsedTime)
{
	for (auto& com : this->pool)
		com.PostUpdate(elapsedTime);
}

void KG::System::DynamicRigidSystem::OnPreRender()
{

}

// void KG::System::DynamicRigidSystem::OnPreRender()
// {
// 
// }

void KG::System::StaticRigidSystem::OnUpdate(float elapsedTime)
{
}

void KG::System::StaticRigidSystem::OnPostUpdate(float elapsedTime)
{
	for (auto& com : this->pool)
		com.PostUpdate(elapsedTime);
}

void KG::System::StaticRigidSystem::OnPreRender()
{
}
