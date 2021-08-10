#include "pch.h"
#include "Network.h"
#include "ClientLobbyComponent.h"

bool KG::Component::CLobbyComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
    switch (type)
    {
    case KG::Packet::PacketType::SC_LOGIN_OK:
    {
        auto* Packet = KG::Packet::PacketCast<KG::Packet::SC_LOGIN_OK>(packet);
        this->id = Packet->lobbyid;
        return true;
    }
    case KG::Packet::PacketType::SC_LOBBY_FULL:
    {
        // *
        // 연결 끊기
        return true;
    }
    case KG::Packet::PacketType::SC_LOBBY_DATA:
    {
        auto* Packet = KG::Packet::PacketCast<KG::Packet::SC_LOBBY_DATA>(packet);
        // *
        // 전부 레디면 바로시작? -> 시작 버튼 필요 X
        // 시작 버튼 -> 시작 버튼을 쓰면 누가 누를지
        for (size_t i = 0; i < PLAYERNUM; i++)
        {
            this->playerInfo[i] = Packet->playerinfo[i];
        }
        return true;
    }
    case KG::Packet::PacketType::SC_GAME_START:
    {
        // 게임 시작
    }
    }
    return false;
}

void KG::Component::CLobbyComponent::SendReadyPacket() {
    KG::Packet::CS_LOBBY_CHANGE Packet;
    Packet.id = this->id;
    Packet.state = LobbyState::Ready;
    this->SendPacket(&Packet);
}

void KG::Component::CLobbyComponent::SendWaitPacket() {
    KG::Packet::CS_LOBBY_CHANGE Packet;
    Packet.id = this->id;
    Packet.state = LobbyState::Wait;
    this->SendPacket(&Packet);
}

void KG::Component::CLobbyComponent::SendSelectPacket(int mapnumber) {
    KG::Packet::CS_SELECT_MAP Packet;
    Packet.mapnum = mapnumber;
    this->SendPacket(&Packet);
}


void KG::Component::CLobbyComponent::OnCreate(KG::Core::GameObject* obj)
{
    this->SetNetObjectId(KG::Server::LOBBY_ID);
    this->network->SetNetworkObject(this->networkObjectId, this);
}

void KG::Component::CLobbyComponent::Update(float elapsedTime)
{
}

char KG::Component::CLobbyComponent::GetLobbyInfo(int num)
{
    return this->playerInfo[num];
}



void KG::Component::CLobbyComponent::SendLoginPacket() {
    KG::Packet::CS_REQ_LOGIN login = {};
    this->SendPacket(&login);
}