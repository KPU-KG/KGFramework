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
				// LOGIN_OK 패킷으로 클라이언트에 로비 번호 부여

				this->PlayerInfo[i] = LobbyState::Wait;
				KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
				for (size_t j = 0; j < PLAYERNUM; j++)
				{
					LobbyDataPacket.playerinfo[j] = this->PlayerInfo[j];
				}
				this->BroadcastPacket(&LobbyDataPacket);
				// 갱신된 로비 정보 전송
				return true;
			}
		}

		KG::Packet::SC_LOBBY_FULL RefusePacket;
		this->SendPacket(sender, &RefusePacket);
		// 로비 자리 없으면 세션 종료
	}
	return true;
	case KG::Packet::PacketType::CS_LOBBY_CHANGE:
	{
		auto* Packet = KG::Packet::PacketCast<KG::Packet::CS_LOBBY_CHANGE>(packet);
		this->PlayerInfo[Packet->id] = Packet->state;
		// 패킷 정보로 로비 정보 갱신

		KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
		for (size_t j = 0; j < PLAYERNUM; j++)
		{
			LobbyDataPacket.playerinfo[j] = this->PlayerInfo[j];
		}
		this->BroadcastPacket(&LobbyDataPacket);
		// 갱신된 로비 정보 전송

		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->PlayerInfo[i] != LobbyState::Ready) {
				return true;
			}
		}
		KG::Packet::SC_GAME_START StartPacket;
		this->BroadcastPacket(&StartPacket);
		// 전부 레디면 시작 패킷 전송
	}
	return true;
	}
	return false;
}