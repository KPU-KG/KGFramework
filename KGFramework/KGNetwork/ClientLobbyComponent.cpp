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
        // ���� ��ư -> ���� ��ư�� ���� ���� ��������?
        for (size_t i = 0; i < PLAYERNUM; i++)
        {
            this->playerInfo[i] = Packet->playerinfo[i];
        }
        return true;
    }
    }
    return false;
}