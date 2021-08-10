#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"

static enum LobbyState
{
	Empty,
	Wait,
	Ready,
};

namespace KG::Component
{
	class DLL CLobbyComponent : public CBaseComponent
	{
	private:
		char playerInfo[PLAYERNUM] = { LobbyState::Empty, }; // 접속 정보
		int id = -1; // 클라 접속 id, 접속 성공 시 변경
		
	public:
		// id 포함한 정보 변경 패킷 전송
		void SendReadyPacket();
		void SendWaitPacket();
		void SendLoginPacket();
		void SendSelectPacket(int mapnumber);
		char GetLobbyInfo(int num);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override; // 로비 정보 변경 처리
	};
	REGISTER_COMPONENT_ID(CLobbyComponent);
}