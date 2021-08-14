#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include <functional>

namespace KG::Component
{
	class TransformComponent;
	class IAnimationControllerComponent;
	class DynamicRigidComponent;
	class ISoundComponent;

	class DLL CCharacterComponent : public CBaseComponent
	{

		KG::Component::TransformComponent* transform = nullptr;
		KG::Component::TransformComponent* rotationTransform = nullptr;
		KG::Component::IAnimationControllerComponent* characterAnimation = nullptr;
		KG::Component::DynamicRigidComponent* physics = nullptr;
		KG::Component::ISoundComponent* sound = nullptr;

		constexpr static float inputRatio = 25.0f;
		constexpr static float inputRetRatio = 5.0f;
		constexpr static float inputMinimum = 0.1f;
		constexpr static float walkBlendingDuration = 0.1f;
		constexpr static float bulletRepeatTime = 0.1f;
		float forwardValue = 0.0f;
		float rightValue = 0.0f;
		float speedValue = 0.75f;
		KG::Packet::INPUTS inputs;
		void ProcessMoveAnim();
		void ProcessMove(float elapsedTime);
		void InterpolatePosition(DirectX::XMFLOAT3 position);
	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
	};
	REGISTER_COMPONENT_ID(CCharacterComponent);
}