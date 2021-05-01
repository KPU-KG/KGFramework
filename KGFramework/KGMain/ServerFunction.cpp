#include <string>
#include "ImguiHelper.h"
#include "KGFramework.h"
#include "GraphicComponent.h"
#include "KGRenderer.h"
#include "Systems.h"
#include "Debug.h"
#include "GameObject.h"
#include "MaterialMatch.h"
#include "LambdaComponent.h"
#include "SceneCameraComponent.h"
#include "IKGServer.h"
#include "ServerPlayerControllerComponent.h"
#include "ServerEnemyControllerComponent.h"
#include "InputManager.h"

void KG::GameFramework::PostServerFunction()
{
	assert(this->networkServer && "server not Init");
	this->scene->AddNetworkCreator(
		KG::Utill::HashString("TeamCharacter"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* comp = this->networkServer->GetNewPlayerComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("TileCube"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* comp = this->networkServer->GetNewPlayerComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("EnemyMech"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent* {
			auto* comp = this->networkServer->GetNewEnemyControllerComponent();
			comp->SetIdleInterval(2);
			comp->SetRotateInterval(3);
			comp->SetSpeed(3);
			comp->SetWanderRange(3);
			comp->SetRaycastCallback([this, comp](KG::Component::RaycastType type, KG::Component::IRigidComponent* other) {
				if (type == KG::Component::RaycastType::BULLET_HIT) {
					comp->HitBullet();
				}
				});
			obj.AddComponent(comp);
			return comp;
		}
	);
}