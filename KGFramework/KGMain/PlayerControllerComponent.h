#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>
namespace KG::Component
{
	class TransformComponent;
	class CameraComponent;
	class AnimationControllerComponent;

	class PlayerControllerComponent : public IComponent
	{
	private:
		TransformComponent* characterTransform = nullptr;
		AnimationControllerComponent* characterAnimation = nullptr;

		CameraComponent* camera = nullptr;
		TransformComponent* cameraTransform = nullptr;
		AnimationControllerComponent* vectorAnimation = nullptr;
		float speedValue = 0.75f;

		// Bullet
		int bulletCount = 30;


	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID(PlayerControllerComponent);

	class PlayerControllerComponentSystem : public KG::System::IComponentSystem<PlayerControllerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;

	};
}