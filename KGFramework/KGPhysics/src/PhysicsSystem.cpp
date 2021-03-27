#include "PhysicsSystem.h"

void KG::System::PhysicsSystem::OnUpdate(float elapsedTime)
{

}

void KG::System::PhysicsSystem::OnPostUpdate(float elapsedTime)
{
	for (auto& com : this->pool)
		com.PostUpdate(elapsedTime);
}

void KG::System::PhysicsSystem::OnPreRender()
{

}
