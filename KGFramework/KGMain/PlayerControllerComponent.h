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

		constexpr static float inputRatio = 10.0f;
		constexpr static float inputRetRatio = 10.0f;
		constexpr static float inputMinimum = 0.1f;
		constexpr static float walkBlendingDuration = 0.1f;
		float forwardValue = 0.0f;
		float rightValue = 0.0f;

		// Bullet
		int bulletCount = 30;


		static constexpr auto anim_soldier_standing = "Soldier@Standing.fbx"_id;
		static constexpr auto anim_soldier_sprint_f = "Soldier@SprintForward.fbx"_id;
		static constexpr auto anim_soldier_sprint_fl = "Soldier@SprintForwardLeft.fbx"_id;
		static constexpr auto anim_soldier_sprint_fr = "Soldier@Standing.SprintForwardRight"_id;
		static constexpr auto anim_soldier_walk_f = "Soldier@WalkForward.fbx"_id;
		static constexpr auto anim_soldier_walk_fl = "Soldier@WalkForwardLeft.fbx"_id;
		static constexpr auto anim_soldier_walk_fr = "Soldier@WalkForwardRight.fbx"_id;
		static constexpr auto anim_soldier_walk_l = "Soldier@WalkLeft.fbx"_id;
		static constexpr auto anim_soldier_walk_r = "Soldier@WalkRight.fbx"_id;
		static constexpr auto anim_soldier_walk_b = "Soldier@WalkBackward.fbx"_id;
		static constexpr auto anim_soldier_walk_bl = "Soldier@WalkBackwardLeft.fbx"_id;
		static constexpr auto anim_soldier_walk_br = "Soldier@WalkBackwardRight.fbx"_id;


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