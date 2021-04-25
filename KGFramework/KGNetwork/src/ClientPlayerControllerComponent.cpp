#include "pch.h"
#include "ClientPlayerControllerComponent.h"
#include "Transform.h"

void KG::Component::CPlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->transform = this->GetGameObject()->GetComponent<TransformComponent>();
}

void KG::Component::CPlayerControllerComponent::Update(float elapsedTime)
{
	KG::Packet::CS_INPUT inputPacket = {};
	auto input = KG::Input::InputManager::GetInputManager();

	//MouseCaptureMode
	//if ( input->IsTouching('9') )
	//{
	//	input->SetMouseCapture(false);
	//}

	//if ( input->IsTouching('0') )
	//{
	//	input->SetMouseCapture(true);
	//}

	inputPacket.stateW = static_cast<unsigned char>(input->GetKeyState('W'));
	inputPacket.stateA = static_cast<unsigned char>(input->GetKeyState('A'));
	inputPacket.stateS = static_cast<unsigned char>(input->GetKeyState('S'));
	inputPacket.stateD = static_cast<unsigned char>(input->GetKeyState('D'));
	inputPacket.stateShift = static_cast<unsigned char>(input->GetKeyState(VK_SHIFT));
	bool shouldSend = inputPacket.stateW != inputCache.stateW;
	shouldSend |= (inputPacket.stateA != inputCache.stateA);
	shouldSend |= (inputPacket.stateS != inputCache.stateS);
	shouldSend |= (inputPacket.stateD != inputCache.stateD);
	shouldSend |= (inputPacket.stateShift != inputCache.stateShift);
	if ( shouldSend )
	{
		this->SendPacket(&inputPacket);
		inputCache = inputPacket;
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
			this->transform->SetPosition(ScenePacket->position);
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
