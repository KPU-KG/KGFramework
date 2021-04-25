#include "pch.h"
#include "ServerPlayerControllerComponent.h"

void KG::Component::SPlayerComponent::InitData(KG::Server::NET_OBJECT_ID managerplayerObjectIds[] , KG::Packet::RawFloat3 managerpositions[]) {
	for (size_t i = 0; i < 4; i++)
	{
		playerObjectIds[i] = managerplayerObjectIds[i];
		positions[i] = managerpositions[i];
	}
}

void KG::Component::SPlayerComponent::OnCreate(KG::Core::GameObject* obj)
{
}

void KG::Component::SPlayerComponent::Update(float elapsedTime)
{
	for (size_t i = 0; i < 4; i++)
	{
		//positions[i] = gameObject->GetComponentWithID(playerObjectIds[i])->GetGameObject()->GetTransform();
		if (inputs[i].stateW) {
			positions[i].z += 0.05;
		}
		if (inputs[i].stateA) {
			positions[i].x -= 0.05;
		}
		if (inputs[i].stateS) {
			positions[i].z -= 0.05;
		}
		if (inputs[i].stateD) {
			positions[i].x += 0.05;
		}
	}

	if (updatetimer < 0.015f)
		updatetimer += elapsedTime;
	else {
		KG::Packet::SC_PLAYER_DATA DataPacket = {};
		for (size_t i = 0; i < 4; i++)
		{
			DataPacket.playerObjectIds[i] = playerObjectIds[i];
			DataPacket.positions[i] = positions[i];
		}
		this->BroadcastPacket(&DataPacket);
		std::cout << "send scene data\n";
		updatetimer = 0;
	}
}

bool KG::Component::SPlayerComponent::OnDrawGUI()
{
	return false;
}

bool KG::Component::SPlayerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::CS_INPUT:
	{
		auto* InputPacket = KG::Packet::PacketCast<KG::Packet::CS_INPUT>(packet);
		for (size_t i = 0; i < 4; i++)
		{
			if (playerObjectIds[i] == InputPacket->header.objectId) {
				inputs[i].stateW = InputPacket->stateW;
				inputs[i].stateA = InputPacket->stateA;
				inputs[i].stateS = InputPacket->stateS;
				inputs[i].stateD = InputPacket->stateD;
				inputs[i].stateShift = InputPacket->stateShift;
			}
		}
		std::cout << "get input" << InputPacket->header.objectId <<
			"w:" << (int)InputPacket->stateW <<
			"a:" << (int)InputPacket->stateA <<
			"s:" << (int)InputPacket->stateS <<
			"d:" << (int)InputPacket->stateD << std::endl;
	}
	return true;
	}
	return false;
}

void KG::Component::SPlayerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::SPlayerComponentSystem::OnPreRender()
{
}
