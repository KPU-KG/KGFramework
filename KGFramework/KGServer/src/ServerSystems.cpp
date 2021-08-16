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

void KG::System::SEnemyCrawlerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SEnemyCrawlerComponentSystem::OnPreRender() 
{
}

void KG::System::SLobbyComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SLobbyComponentSystem::OnPreRender()
{
}

void KG::System::SLobbyComponentSystem::DisconnectPlayer(KG::Server::SESSION_ID playerID)
{
	for (auto& com : *this) {
		com.DisconnectLobbyPlayer(playerID);
	}
}


void KG::System::SCubeAreaRedSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SCubeAreaRedSystem::OnPreRender()
{
}

void KG::System::SCrawlerMissileComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SCrawlerMissileComponentSystem::OnPreRender()
{
}

void KG::System::SEnemyTurretComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::System::SEnemyTurretComponentSystem::OnPreRender()
{
}
