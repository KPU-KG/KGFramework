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
            {
                auto* uiObj = this->scene->CallPreset("2DUI"_id);
                uiObj->tag = KG::Utill::HashString("DIGIT_0");
                auto* r2d = uiObj->GetComponent<KG::Component::IRender2DComponent>();
                r2d->material2D.materialId = "digit_0"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 0);
                r2d->transform2D.size = DirectX::XMFLOAT2(0.2, 0.2);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::RIGHT_BOTTOM;
                r2d->transform2D.localPivot = KG::Component::RectPivot::RIGHT_BOTTOM;
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }
            {
                auto* uiObj = this->scene->CallPreset("2DUI"_id);
                uiObj->tag = KG::Utill::HashString("DIGIT_1");
                auto* r2d = uiObj->GetComponent<KG::Component::IRender2DComponent>();
                r2d->material2D.materialId = "digit_0"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(-0.04, 0);
                r2d->transform2D.size = DirectX::XMFLOAT2(0.2, 0.2);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::RIGHT_BOTTOM;
                r2d->transform2D.localPivot = KG::Component::RectPivot::RIGHT_BOTTOM;
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }
            {
                auto* uiObj = this->scene->CallPreset("2DUI"_id);
                uiObj->tag = KG::Utill::HashString("CrossHair");
                auto* r2d = uiObj->GetComponent<KG::Component::IRender2DComponent>();
                r2d->material2D.materialId = "crosshair"_id;
                r2d->transform2D.size = DirectX::XMFLOAT2(0.075, 0.075);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }
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

