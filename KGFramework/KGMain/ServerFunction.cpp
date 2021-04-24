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
}