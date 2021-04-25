#include "pch.h"
#include "ClientCharacterComponent.h"
#include "Transform.h"

void KG::Component::CCharacterComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->transform = this->GetGameObject()->GetComponent<TransformComponent>();
}

void KG::Component::CCharacterComponent::Update(float elapsedTime)
{
}

bool KG::Component::CCharacterComponent::OnDrawGUI()
{
	return false;
}

bool KG::Component::CCharacterComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch ( type )
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

