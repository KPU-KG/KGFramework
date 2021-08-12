#include "pch.h"
#include "ServerLobbyComponent.h"
#include "KGServer.h"
#include "Scene.h"


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
	for (auto &p : this->playerinfo)
	{
		if (p.id == playerId) {
			p.state = LobbyState::Empty;
			p.id = -1;
		}
	}

	KG::Packet::SC_LOBBY_DATA Packet;
	for (size_t i = 0; i < PLAYERNUM; i++)
	{
		Packet.playerinfo[i] = (char)this->playerinfo[i].state;
		Packet.mapnum = this->mapnum;
	}
	this->BroadcastPacket(&Packet);
}

bool KG::Component::SLobbyComponent::OnDrawGUI()
{
    if (ImGui::ComponentHeader<SLobbyComponent>())
    {
        for (size_t i = 0; i < 4; i++)
        {
            const char* txt[] = { "Empty","Wait","Ready" };
            ImGui::BulletText("User %d : %s", i + 1, txt[this->playerinfo[i].state]);
        }
    }
    return false;
}

bool KG::Component::SLobbyComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		/*auto* scene = this->gameObject->GetScene();
		scene->LoadScene("Resource/Scenes/SceneData_86_client.xml");*/
		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->playerinfo[i].state == LobbyState::Empty) {
				this->server->SetSessionState(sender, KG::Server::PLAYER_STATE::PLAYER_STATE_INGAME);
				KG::Packet::SC_LOGIN_OK Packet;
				Packet.lobbyid = i;
				this->SendPacket(sender, &Packet);
				// LOGIN_OK 패킷으로 클라이언트에 로비 번호 부여

				this->playerinfo[i].state = LobbyState::Wait;
				this->playerinfo[i].id = sender;
				KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
				for (size_t j = 0; j < PLAYERNUM; j++)
				{
					LobbyDataPacket.playerinfo[j] = this->playerinfo[j].state;
					LobbyDataPacket.mapnum = this->mapnum;
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
		this->playerinfo[Packet->id].state = Packet->state;
		// 패킷 정보로 로비 정보 갱신

		KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
		for (size_t j = 0; j < PLAYERNUM; j++)
		{
			LobbyDataPacket.playerinfo[j] = this->playerinfo[j].state;
			LobbyDataPacket.mapnum = this->mapnum;
		}
		this->BroadcastPacket(&LobbyDataPacket);
		// 갱신된 로비 정보 전송
		
		int playernum = 0; // 로비 접속 플레이어 확인

		for (size_t i = 0; i < PLAYERNUM; i++)
		{
			if (this->playerinfo[i].state == LobbyState::Wait) {
				return true;
			}
			if (this->playerinfo[i].state == LobbyState::Ready) {
				playernum += 1;
			}
		}

		this->server->SetPlayerNum(playernum);

		KG::Packet::SC_GAME_START StartPacket;
		StartPacket.mapnum = this->mapnum;
		this->BroadcastPacket(&StartPacket);
		//this->server->isPlay = true;
		// 전부 레디면 시작 + 시작 패킷 전송
	}
	return true;
	case KG::Packet::PacketType::CS_SELECT_MAP: // 맵 선택 패킷 -> 맵 번호 로비 서버에 저장
	{
		auto* Packet = KG::Packet::PacketCast<KG::Packet::CS_SELECT_MAP>(packet);
		this->mapnum = Packet->mapnum;

		KG::Packet::SC_LOBBY_DATA LobbyDataPacket;
		for (size_t j = 0; j < PLAYERNUM; j++)
		{
			LobbyDataPacket.playerinfo[j] = this->playerinfo[j].state;
			LobbyDataPacket.mapnum = this->mapnum;
		}
		this->BroadcastPacket(&LobbyDataPacket);
		
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
