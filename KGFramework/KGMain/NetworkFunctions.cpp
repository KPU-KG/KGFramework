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
#include "IKGNetwork.h"
#include "ClientCharacterComponent.h"
#include "ClientPlayerControllerComponent.h"
#include "ClientEnemyControllerComponent.h"
#include "InputManager.h"


void KG::GameFramework::PostNetworkFunction()
{
	assert(this->networkClient && "network not Init");
	this->scene->AddNetworkCreator(
		KG::Utill::HashString("TeamCharacter"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* comp = this->networkClient->GetNewCharacterComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("PlayerCharacter"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* comp = this->networkClient->GetNewPlayerControllerComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("EnemyMech"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
			auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
			obj.AddComponent(comp);
			return comp;
		}
	);
}

