#include "pch.h"
#include "ServerSystems.h"

// void KG::System::SEnemyControllerComponentSystem::OnPostUpdate(float elapsedTime)
// {
// 
// }
// 
// void KG::System::SEnemyControllerComponentSystem::OnPreRender()
// {
// 
// }

void KG::System::SGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SGameManagerComponentSystem::OnPreRender()
{
}

void KG::System::SPlayerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SPlayerComponentSystem::OnPreRender()
{
}

void KG::System::EnemyGeneratorSystem::OnPostUpdate(float elapsedTime)
{

}

void KG::System::EnemyGeneratorSystem::OnPreRender()
{

}

void KG::System::EnemyGeneratorSystem::DestroyGameObject()
{
	for (auto& com : *this) {
		com.Destroy();
	}
}

void KG::System::SProjectileComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SProjectileComponentSystem::OnPreRender()
{
}

void KG::System::SEnemyMechComponentSystem::OnPostUpdate(float elapsedTime)
{

}

void KG::System::SEnemyMechComponentSystem::OnPreRender()
{

}
