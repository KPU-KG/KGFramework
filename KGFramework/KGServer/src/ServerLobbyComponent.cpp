#include "pch.h"
#include "ServerLobbyComponent.h"
#include "KGServer.h"


KG::Component::SLobbyComponent::SLobbyComponent()
{

}

//bool KG::Component::SLobbyComponent::CheckReady() {
//	for (size_t i = 0; i < PLAYERNUM; i++)
//	{
//		if (this->playerinfo[i].state == LobbyState::Wait) {
//			return false;
//		}
//	}
//	return true;
//}

void KG::Component::SLobbyComponent::DisconnectLobbyPlayer(KG::Server::SESSION_ID playerId) {
	std::cout << "disconnect lobby id: " << playerId << "\n";
	for (size_t i = 0; i < 4; i++)
	{
		std::cout << this->playerinfo[i].state << "\n";
	}
	for (auto p : this->playerinfo)
	{
		if (p.id == playerId) {
			p.state = LobbyState::Empty;
			p.id = -1;
			std::cout << "disconnect\n";
		}
	}
}

bool KG::Component::SLobbyComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->playerinfo[i].state == LobbyState::Empty) {
				this->server->SetSessionState(sender, KG::Server::PLAYER_STATE::PLAYER_STATE_INGAME);
				KG::Packet::SC_LOGIN_OK Packet;
				Packet.lobbyid = i;
				this->SendPacket(sender, &Packet);
				// LOGIN_OK ��Ŷ���� Ŭ���̾�Ʈ�� �κ� ��ȣ �ο�

				this->playerinfo[i].state = LobbyState::Wait;
				this->playerinfo[i].id = sender;
				KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
				for (size_t j = 0; j < PLAYERNUM; j++)
				{
					LobbyDataPacket.playerinfo[j] = this->playerinfo[j].state;
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
		this->playerinfo[Packet->id].state = Packet->state;
		// ��Ŷ ������ �κ� ���� ����

		KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
		for (size_t j = 0; j < PLAYERNUM; j++)
		{
			LobbyDataPacket.playerinfo[j] = this->playerinfo[j].state;
		}
		this->BroadcastPacket(&LobbyDataPacket);
		// ���ŵ� �κ� ���� ����

		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->playerinfo[i].state != LobbyState::Ready) {
				return true;
			}
		}

		// ����� �� ��ȣ�� �� �ε�

		KG::Packet::SC_GAME_START StartPacket;
		this->BroadcastPacket(&StartPacket);
		this->server->isPlay = true;
		// ���� ����� ���� + ���� ��Ŷ ����
	}
	return true;
	case KG::Packet::PacketType::CS_SELECT_MAP: // �� ���� ��Ŷ -> �� ��ȣ �κ� ������ ����
	{
		auto* Packet = KG::Packet::PacketCast<KG::Packet::CS_SELECT_MAP>(packet);
		this->mapnum = Packet->mapnum;
	}
	return true;
	}
	
	return false;
	
}


void KG::Component::SLobbyComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::LOBBY_ID);
	this->server->SetServerObject(this->networkObjectId, this);
}

