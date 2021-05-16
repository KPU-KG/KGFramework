#include "pch.h"
#include "ClientProjectileComponent.h"
#include "Transform.h"

KG::Component::CProjectileComponent::CProjectileComponent()
{
}

void KG::Component::CProjectileComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);
	this->transform = this->gameObject->GetTransform();
	this->transform->SetScale(0.1, 0.1, 0.1);
}

void KG::Component::CProjectileComponent::Update(float elapsedTime)
{
}

bool KG::Component::CProjectileComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch (type) {
	case KG::Packet::PacketType::SC_MOVE_OBJECT:
	{
		auto* p = KG::Packet::PacketCast<KG::Packet::SC_MOVE_OBJECT>(packet);
		this->transform->SetPosition(p->position);
		this->transform->SetRotation(p->rotation);
		return true;
	}
	case KG::Packet::PacketType::SC_REMOVE_OBJECT:
	{
		this->gameObject->Destroy();
		return true;
	}
	return false;
	}
}

bool KG::Component::CProjectileComponent::OnDrawGUI()
{
	return false;
}
