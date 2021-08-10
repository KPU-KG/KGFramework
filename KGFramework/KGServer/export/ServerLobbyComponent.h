#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"

static enum LobbyState
{
	Empty,
	Wait,
	Ready,
};

struct PlayerInfo {
	char state = LobbyState::Empty;
	KG::Server::SESSION_ID id = -1;
};
namespace KG::Component
{
	class DLL SLobbyComponent : public SBaseComponent
	{
	private:
		// 플레이어 접속 상태
		PlayerInfo playerinfo[PLAYERNUM];
		int mapnum = 0;
		// *
		// 연결 끊어졌을 때 Empty로 변경
		
	public:
		SLobbyComponent();
		void DisconnectLobbyPlayer(KG::Server::SESSION_ID playerId);
		// bool CheckReady();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Destroy() override {};
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		//virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
		
	};
	REGISTER_COMPONENT_ID(SLobbyComponent);
}

