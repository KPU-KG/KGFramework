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
#include "ClientProjectileComponent.h"
#include "ClientCubeAreaRedComponent.h"
#include "InputManager.h"


void KG::GameFramework::PostNetworkFunction()
{
	assert(this->networkClient && "network not Init");
	this->scene->AddNetworkCreator(
		KG::Utill::HashString("TeamCharacter"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{
            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 2.25f);
                r2d->transform2D.size = DirectX::XMFLOAT2(2, 0.25);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(0, 1, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }
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

            {
                auto* uiObj = this->scene->CallPreset("2DUI"_id);
                uiObj->tag = KG::Utill::HashString("HPUI");
                auto* r2d = uiObj->GetComponent<KG::Component::IRender2DComponent>();
                r2d->material2D.materialId = "UIWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0.1f, 0.1f);
                r2d->transform2D.size = DirectX::XMFLOAT2(0.7, 0.075);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::LEFT_BOTTOM;
                r2d->transform2D.localPivot = KG::Component::RectPivot::LEFT_BOTTOM;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }
			auto* comp = this->networkClient->GetNewPlayerControllerComponent();
			obj.AddComponent(comp);
			return comp;
		}
	);

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("Projectile"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            auto* comp = this->networkClient->GetNewProjectileComponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("Missile"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            auto* comp = this->networkClient->GetNewProjectileComponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("CrawlerMissile"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            auto* comp = this->networkClient->GetNewProjectileComponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

	this->scene->AddNetworkCreator(
		KG::Utill::HashString("EnemyMech"),
		[this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
		{

            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 7);
                r2d->transform2D.size = DirectX::XMFLOAT2(5, 0.75);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }

			auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
			obj.AddComponent(comp);

			return comp;
		}
	);

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("Turret"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 5);
                r2d->transform2D.size = DirectX::XMFLOAT2(5, 0.75);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }

            auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("EnemyMechGreen"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {

            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 7);
                r2d->transform2D.size = DirectX::XMFLOAT2(5, 0.75);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }

            auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("EnemyMechMetal"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {

            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 7);
                r2d->transform2D.size = DirectX::XMFLOAT2(5, 0.75);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }

            auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("EnemyCrawler"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            {
                auto* uiObj = this->scene->CallPreset("SPRITEUI"_id);
                uiObj->tag = KG::Utill::HashString("HPBAR");
                auto* r2d = uiObj->GetComponent<KG::Component::IRenderSpriteComponent>();
                r2d->material2D.materialId = "SpriteWhite"_id;
                r2d->transform2D.position = DirectX::XMFLOAT2(0, 12);
                r2d->transform2D.size = DirectX::XMFLOAT2(10, 1.5);
                r2d->transform2D.parentPivot = KG::Component::RectPivot::CENTER;
                r2d->transform2D.localPivot = KG::Component::RectPivot::CENTER;
                r2d->material2D.color = KG::Utill::Color(1, 0, 0, 1);
                r2d->ReloadRender();
                obj.GetTransform()->AddChild(uiObj->GetTransform());
            }

            auto* comp = this->networkClient->GetNewEnemyControllerOomponent();
            obj.AddComponent(comp);

            return comp;
        }
    );

    this->scene->AddNetworkCreator(
        KG::Utill::HashString("CubeAreaRed"),
        [this](KG::Core::GameObject& obj) -> KG::Component::IComponent*
        {
            auto* comp = this->networkClient->GetNewCubeAreaRedComponent();
            obj.AddComponent(comp);

            return comp;
        }
    );
}

