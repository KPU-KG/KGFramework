#include "pch.h"
#include "ClientBaseComponent.h"

void KG::Component::CBaseComponent::SetNetObjectId(KG::Server::NET_OBJECT_ID id)
{
	this->networkObjectId = id;
}

void KG::Component::CBaseComponent::SetNetworkInstance(KG::Server::Network* network)
{
	this->network = network;
}

void KG::Component::CBaseComponent::SendPacket(unsigned char* packet)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(packet);
	header->objectId = this->networkObjectId;
}

bool KG::Component::CBaseComponent::ProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	return this->OnProcessPacket(packet, type);
}
