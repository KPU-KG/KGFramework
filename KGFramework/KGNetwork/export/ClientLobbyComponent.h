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
		char playerInfo[PLAYERNUM] = { LobbyState::Empty, }; // ���� ����
		int id = -1; // Ŭ�� ���� id, ���� ���� �� ����
		int mapnum = 0;
	public:
		// id ������ ���� ���� ��Ŷ ����
		void SendReadyPacket();
		void SendWaitPacket();
		void SendLoginPacket();
		void SendSelectPacket(int mapnumber);
		char GetLobbyInfo(int num);
		int GetMap();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override; // �κ� ���� ���� ó��
	};
	REGISTER_COMPONENT_ID(CLobbyComponent);
}