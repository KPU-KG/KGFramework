#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include <functional>

namespace KG::Component
{
	class DLL CGameManagerComponent : public CBaseComponent
	{
	public:
		KG::Packet::CS_INPUT inputPacket = {};
		float updatetimer = 0;
        std::function<void()> clearFunction;
		void GameClear();
        void PostGameClearFunction(const std::function<void()>& func);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();

		// CBaseComponent을(를) 통해 상속됨
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;

        void SendLoginPacket();
	};
	REGISTER_COMPONENT_ID(CGameManagerComponent);
}