#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"

namespace KG::Physics {
	class IPhysicsScene;
}

static enum LobbyState
{
	Empty,
	Wait,
	Ready,
};

namespace KG::Component
{
	class DLL SLobbyComponent : public SBaseComponent
	{
	private:
		// 플레이어 접속 상태
		char PlayerInfo[PLAYERNUM] = { LobbyState::Empty, };
		// *
		// 연결 끊어졌을 때 Wait로 변경
	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void Destroy() override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender); // sendpacket
	};
	REGISTER_COMPONENT_ID(SLobbyComponent);
}

