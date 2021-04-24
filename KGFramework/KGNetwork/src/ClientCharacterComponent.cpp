#include "pch.h"
#include "ClientCharacterComponent.h"

void KG::Component::CCharacterComponent::OnCreate(KG::Core::GameObject* obj)
{
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
	return false;
}

void KG::Component::CCharacterComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CCharacterComponentSystem::OnPreRender()
{
}
