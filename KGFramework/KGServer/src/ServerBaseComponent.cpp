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

void KG::Component::SBaseComponent::BroadcastPacket(void* packet, KG::Server::SESSION_ID ignoreId)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(packet);
	header->objectId = this->networkObjectId;
	this->server->BroadcastPacket(packet, ignoreId);
}

void KG::Component::SBaseComponent::SendPacket(KG::Server::SESSION_ID sessionId, void* packet)
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
	/*switch (type)
	{
	case KG::Packet::PacketType::CS_INPUT:
	{
		auto* InputPacket = KG::Packet::PacketCast<KG::Packet::CS_INPUT>(packet);
		this->server->inputs[InputPacket->header.objectId].stateW = InputPacket->stateW;
		this->server->inputs[InputPacket->header.objectId].stateA = InputPacket->stateA;
		this->server->inputs[InputPacket->header.objectId].stateS = InputPacket->stateS;
		this->server->inputs[InputPacket->header.objectId].stateD = InputPacket->stateD;
		this->server->inputs[InputPacket->header.objectId].stateShift = InputPacket->stateShift;
		std::cout << "get input" << InputPacket->header.objectId << std::endl;
	}
	return true;
	}*/
	return false;
}
