#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include "InputManager.h"
#include <functional>

namespace KG::Component
{
	class TransformComponent;
	class CameraComponent;
	class AnimationControllerComponent;
	class DynamicRigidComponent;
	class ParticleEmitterComponent;

	class DLL CPlayerControllerComponent : public CBaseComponent
	{
	private:
		TransformComponent* characterTransform = nullptr;
		AnimationControllerComponent* characterAnimation = nullptr;
		CameraComponent* camera = nullptr;
		TransformComponent* cameraTransform = nullptr;
		AnimationControllerComponent* vectorAnimation = nullptr;
		DynamicRigidComponent* physics = nullptr;
		ParticleEmitterComponent* particleGen = nullptr;

		float speedValue = 0.75f;

		constexpr static float inputRatio = 25.0f;
		constexpr static float inputRetRatio = 5.0f;
		constexpr static float inputMinimum = 0.1f;
		constexpr static float walkBlendingDuration = 0.1f;
		constexpr static float bulletRepeatTime = 0.1f;
		float forwardValue = 0.0f;
		float rightValue = 0.0f;

		bool isActive = true;

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
		void InternalUpdate(float elapsedTime);
		void SendUpdate(float elapsedTime);
	public:
		KG::Input::InputManager* input;
		static constexpr float sendPacketTimeInterval = 1 / 20.0f;
		float sendPacketTimer = 0;
		int id = -1;
		KG::Packet::CS_INPUT inputCache;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();

		// CBaseComponent을(를) 통해 상속됨
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
	};
	REGISTER_COMPONENT_ID(CPlayerControllerComponent);
}