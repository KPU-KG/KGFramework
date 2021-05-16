#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>
#include <shared_mutex>

namespace KG::Physics {
	class IPhysicsScene;
}


namespace KG::Component
{
	class TransformComponent;
	class CameraComponent;
	class AnimationControllerComponent;
	class DynamicRigidComponent;

	class DLL SPlayerComponent : public SBaseComponent
	{
	private:
		static constexpr float packetInterval = 1 / 60.0f;
		float packetSendTimer = 0;
		KG::Packet::CS_INPUT inputs;
		KG::Component::TransformComponent* trasnform = nullptr;
		KG::Component::TransformComponent* rotationTrasnform = nullptr;
		KG::Component::DynamicRigidComponent* physics = nullptr;
		KG::Physics::IPhysicsScene* physicsScene = nullptr;

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
        static constexpr int MAX_HP = 30;
        int hpPoint = MAX_HP;

		bool reloadFlag = false;

		void SendSyncPacket();
		void ProcessMove(float elapsedTime);
	public:
		std::shared_mutex playerInfoLock;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void Destroy() override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual void SetPhysicsScene(KG::Physics::IPhysicsScene* physicsScene)
		{
			this->physicsScene = physicsScene;
		}
		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
		void HitBullet(int damage);
	};
	REGISTER_COMPONENT_ID(SPlayerComponent);



}