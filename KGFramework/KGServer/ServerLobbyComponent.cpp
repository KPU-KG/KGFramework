#include "pch.h"
#include "ServerLobbyComponent.h"
#include "KGServer.h"

bool KG::Component::SLobbyComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->PlayerInfo[i]) {
				KG::Packet::SC_LOGIN_OK Packet;
				Packet.lobbyid = i;
				this->SendPacket(sender, &Packet);
				// LOGIN_OK ��Ŷ���� Ŭ���̾�Ʈ�� �κ� ��ȣ �ο�

				this->PlayerInfo[i] = LobbyState::Wait;
				KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
				for (size_t j = 0; j < PLAYERNUM; j++)
				{
					LobbyDataPacket.playerinfo[j] = this->PlayerInfo[j];
				}
				this->BroadcastPacket(&LobbyDataPacket);
				// ���ŵ� �κ� ���� ����
				return true;
			}
		}

		KG::Packet::SC_LOBBY_FULL RefusePacket;
		this->SendPacket(sender, &RefusePacket);
		// �κ� �ڸ� ������ ���� ����
	}
	return true;
	case KG::Packet::PacketType::CS_LOBBY_CHANGE:
	{
		auto* Packet = KG::Packet::PacketCast<KG::Packet::CS_LOBBY_CHANGE>(packet);
		this->PlayerInfo[Packet->id] = Packet->state;
		// ��Ŷ ������ �κ� ���� ����

		KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
		for (size_t j = 0; j < PLAYERNUM; j++)
		{
			LobbyDataPacket.playerinfo[j] = this->PlayerInfo[j];
		}
		this->BroadcastPacket(&LobbyDataPacket);
		// ���ŵ� �κ� ���� ����

		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->PlayerInfo[i] != LobbyState::Ready) {
				return true;
			}
		}
		KG::Packet::SC_GAME_START StartPacket;
		this->BroadcastPacket(&StartPacket);
		// ���� ����� ���� ��Ŷ ����
	}
	return true;
	}
	return false;
}