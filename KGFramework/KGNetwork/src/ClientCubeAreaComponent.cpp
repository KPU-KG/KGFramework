#include "pch.h"
#include "ClientCubeAreaRedComponent.h"
#include "Transform.h"

KG::Component::CCubeAreaRedComponent::CCubeAreaRedComponent()
{

}

void KG::Component::CCubeAreaRedComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);
	this->transform = this->gameObject->GetTransform();
}

void KG::Component::CCubeAreaRedComponent::Update(float elapsedTime)
{
}

bool KG::Component::CCubeAreaRedComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch (type) {
		// scale 추가 예정
	case KG::Packet::PacketType::SC_SCALE_OBJECT:
	{
		auto p = KG::Packet::PacketCast<KG::Packet::SC_SCALE_OBJECT>(packet);
		this->transform->SetScale(p->scale);

		return true;
	}
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
	}
	return false;
}

bool KG::Component::CCubeAreaRedComponent::OnDrawGUI()
{
	return false;
}
