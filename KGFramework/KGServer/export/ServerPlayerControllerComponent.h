#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>
#include <shared_mutex>

namespace KG::Component
{
	class TransformComponent;

	class DLL SPlayerComponent : public SBaseComponent
	{
	private:
		static constexpr float packetInterval = 1 / 20.0f;
		float packetSendTimer = 0;
		KG::Packet::CS_INPUT inputs;
		KG::Component::TransformComponent* trasnform = nullptr;

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

		void SendSyncPacket();
		void ProcessMove(float elapsedTime);
	public:
		std::shared_mutex playerInfoLock;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
	};
	REGISTER_COMPONENT_ID(SPlayerComponent);



}