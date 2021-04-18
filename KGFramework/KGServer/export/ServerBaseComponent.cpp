#include "pch.h"
#include "ServerBaseComponent.h"
#include "KGServer.h"

void KG::Component::SBaseComponent::SetNetObjectId(KG::Server::NET_OBJECT_ID id)
{
	this->networkObjectId = id;
}

inline void KG::Component::SBaseComponent::SetServerInstance(KG::Server::Server* server)
{
	this->server = server;
}

void KG::Component::SBaseComponent::BroadcastPacket(void* packet, KG::Server::SESSION_ID sessionId)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(packet);
	header->objectId = this->networkObjectId;
	this->server->BroadcastPacket(packet, sessionId);
}

void KG::Component::SBaseComponent::SendPacket(KG::Server::SESSION_ID sessionId, unsigned char* packet)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(packet);
	header->objectId = this->networkObjectId;
	this->server->SendPacket(sessionId, packet);
}

bool KG::Component::SBaseComponent::ProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return this->OnProcessPacket(packet, type, sender);
}

bool KG::Component::SBaseComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}
