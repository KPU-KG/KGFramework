#include "pch.h"
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
        // ���� ����
        return true;
    }
    case KG::Packet::PacketType::SC_LOBBY_DATA:
    {
        auto* Packet = KG::Packet::PacketCast<KG::Packet::SC_LOBBY_DATA>(packet);
        // *
        // ���� ����� �ٷν���? -> ���� ��ư �ʿ� X
        // ���� ��ư -> ���� ��ư�� ���� ���� ������
        for (size_t i = 0; i < PLAYERNUM; i++)
        {
            this->playerInfo[i] = Packet->playerinfo[i];
        }
        return true;
    }
    case KG::Packet::PacketType::SC_GAME_START:
    {
        // ���� ����
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


void KG::Component::CLobbyComponent::OnCreate(KG::Core::GameObject* obj)
{
}

void KG::Component::CLobbyComponent::Update(float elapsedTime)
{
}
