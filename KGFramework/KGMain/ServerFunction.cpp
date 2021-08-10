#include <string>
#include "ImguiHelper.h"
#include "KGFramework.h"
#include "IGraphicComponent.h"
#include "KGRenderer.h"
#include "Systems.h"
#include "Debug.h"
#include "GameObject.h"
#include "MaterialMatch.h"
#include "LambdaComponent.h"
#include "SceneCameraComponent.h"
#include "IKGServer.h"
#include "ServerPlayerControllerComponent.h"
#include "ServerGameManagerComponent.h"
#include "ServerEnemyMechComponent.h"
#include "ServerEnemyCrawlerComponent.h"
#include "ServerProjectileComponent.h"
#include "ServerCubeAreaRed.h"
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
		KG::Utill::HashString("Projectile"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* phy = this->physics->GetNewDynamicRigidComponent();
			auto& box = phy->GetCollisionBox();
			box.position = { 0, 0, 0 };
			box.scale = { 1,1,1 };
			phy->SetApply(true);
			phy->AddFilterGroup(KG::Component::FilterGroup::eBULLET, KG::Component::FilterGroup::eENEMY);
			phy->AddFilterGroup(KG::Component::FilterGroup::eNONE, KG::Component::FilterGroup::eBULLET);
			obj.AddComponent(phy);

			auto* comp = this->networkServer->GetNewProjectileComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("Missile"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* phy = this->physics->GetNewDynamicRigidComponent();
			auto& box = phy->GetCollisionBox();
			box.position = { 0, 0, 0 };
			box.scale = { 1,1,3 };
			phy->SetApply(true);
			phy->AddFilterGroup(KG::Component::FilterGroup::eBULLET, KG::Component::FilterGroup::eENEMY);
			phy->AddFilterGroup(KG::Component::FilterGroup::eNONE, KG::Component::FilterGroup::eBULLET);
			obj.AddComponent(phy);

			auto* comp = this->networkServer->GetNewProjectileComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("CrawlerMissile"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* phy = this->physics->GetNewDynamicRigidComponent();
			auto& box = phy->GetCollisionBox();
			box.position = { 0, 0, 0 };
			box.scale = { 1,1,3 };
			phy->SetApply(true);
			phy->AddFilterGroup(KG::Component::FilterGroup::eBULLET, KG::Component::FilterGroup::eENEMY);
			phy->AddFilterGroup(KG::Component::FilterGroup::eNONE, KG::Component::FilterGroup::eBULLET);
			obj.AddComponent(phy);

			auto* comp = this->networkServer->GetNewCrawlerMissileComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("EnemyMech"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent* {

			auto* phy = this->physics->GetNewDynamicRigidComponent();
			KG::Component::CollisionBox box;
			box.position = { 0, 3, 0 };
			box.scale = { 4,6,4 };
			phy->SetCollisionBox(box);
			phy->SetApply(true);
			phy->AddFilterGroup(KG::Component::FilterGroup::eENEMY, KG::Component::FilterGroup::eNONE);
			obj.AddComponent(phy);

			auto* comp = this->networkServer->GetNewEnemyMechComponent();
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

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("EnemyCrawler"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent* {

			auto* phy = this->physics->GetNewDynamicRigidComponent();
			KG::Component::CollisionBox box;
			box.position = { 0, 3, 0 };
			box.scale = { 4,6,4 };
			phy->SetCollisionBox(box);
			phy->SetApply(true);
			phy->AddFilterGroup(KG::Component::FilterGroup::eENEMY, KG::Component::FilterGroup::eNONE);
			obj.AddComponent(phy);



			auto* comp = this->networkServer->GetNewEnemyCrawlerComponent();

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

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("CubeAreaRed"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent* {
			auto* comp = this->networkServer->GetNewCubeAreaRedComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);
}