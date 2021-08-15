#include "pch.h"
#include "ClientProjectileComponent.h"
#include "Transform.h"
#include "ISoundComponent.h"

KG::Component::CProjectileComponent::CProjectileComponent()
{
}

void KG::Component::CProjectileComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);
	this->transform = this->gameObject->GetTransform();
	this->sound = this->gameObject->GetComponent<ISoundComponent>();
	if (this->sound)
	{
        this->sound->Play3DSound(ENEMY_SOUND::LAUNCH, this->transform->GetWorldPosition());
        //this->sound->PlayEffectiveSound(ENEMY_SOUND::LAUNCH);
    }
}

void KG::Component::CProjectileComponent::Update(float elapsedTime)
{
}

void KG::Component::CProjectileComponent::OnDestroy()
{
	if (this->sound)
	{
        this->sound->Play3DSound(ENEMY_SOUND::EXPLOSION, this->transform->GetWorldPosition());
    }
	IComponent::OnDestroy();
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
