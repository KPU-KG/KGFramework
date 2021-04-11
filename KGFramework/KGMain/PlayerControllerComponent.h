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
	class DynamicRigidComponent;

	class PlayerControllerComponent : public IComponent
	{
	private:
		TransformComponent* characterTransform = nullptr;
		AnimationControllerComponent* characterAnimation = nullptr;

		CameraComponent* camera = nullptr;
		TransformComponent* cameraTransform = nullptr;
		AnimationControllerComponent* vectorAnimation = nullptr;
		float speedValue = 0.75f;

		constexpr static float inputRatio = 25.0f;
		constexpr static float inputRetRatio = 5.0f;
		constexpr static float inputMinimum = 0.1f;
		constexpr static float walkBlendingDuration = 0.1f;
		constexpr static float bulletRepeatTime = 0.1f;
		float forwardValue = 0.0f;
		float rightValue = 0.0f;

		// Bullet
		int bulletCount = 30;

		bool reloadFlag = false;

		void ProcessMove(float elapsedTime);
		void ProcessMoveAnimation(float elapsedTime);
		void ProcessShoot(float elapsedTime);
		void ProcessMouse(float elapsedTime);
		void TryShoot(float elapsedTime);
		void TryReload(float elapsedTime);
		bool CheckReloading();

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