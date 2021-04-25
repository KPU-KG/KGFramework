#include "pch.h"
#include "ClientPlayerControllerComponent.h"
#include "Transform.h"

void KG::Component::CPlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
}

void KG::Component::CPlayerControllerComponent::Update(float elapsedTime)
{
	KG::Packet::CS_INPUT inputPacket = {};
	auto input = KG::Input::InputManager::GetInputManager();
	if (input->IsTouching('W'))
		inputPacket.stateW = true;
	else
		inputPacket.stateW = false;
	if (input->IsTouching('S'))
		inputPacket.stateS = true;
	else
		inputPacket.stateS = false;
	if (input->IsTouching('D'))
		inputPacket.stateD = true;
	else
		inputPacket.stateD = false;
	if (input->IsTouching('A'))
		inputPacket.stateA = true;
	else
		inputPacket.stateA = false;
	inputPacket.stateShift = false;
	if (updatetimer < 0.015f)
		updatetimer += elapsedTime;
	else {
		SendPacket(&inputPacket);
		updatetimer = 0;
	}
}

bool KG::Component::CPlayerControllerComponent::OnDrawGUI()
{
	return false;
}

bool KG::Component::CPlayerControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch (type)
	{
	case KG::Packet::PacketType::SC_PLAYER_DATA:
	{
		auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_DATA>(packet);

		for (size_t i = 0; i < 4; i++)
		{
			if (ScenePacket->playerObjectIds[i] == id) {
				std::cout << "scene data recv" << std::endl;
				transform->SetPosition(ScenePacket->positions[i]);
			}
		}
		return true;
	}
	}
	return false;
}

void KG::Component::CPlayerControllerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CPlayerControllerComponentSystem::OnPreRender()
{
}
