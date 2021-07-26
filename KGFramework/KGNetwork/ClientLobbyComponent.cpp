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
        // 연결 끊기
        return true;
    }
    case KG::Packet::PacketType::SC_LOBBY_DATA:
    {
        auto* Packet = KG::Packet::PacketCast<KG::Packet::SC_LOBBY_DATA>(packet);
        // *
        // 전부 레디면 바로시작? -> 시작 버튼 필요 X
        // 시작 버튼 -> 시작 버튼을 쓰면 누가 누르는지?
        for (size_t i = 0; i < PLAYERNUM; i++)
        {
            this->playerInfo[i] = Packet->playerinfo[i];
        }
        return true;
    }
    }
    return false;
}