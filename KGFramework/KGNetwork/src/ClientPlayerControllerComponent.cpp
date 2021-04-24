#include "pch.h"
#include "ClientPlayerControllerComponent.h"

void KG::Component::CPlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
}

void KG::Component::CPlayerControllerComponent::Update(float elapsedTime)
{
}

bool KG::Component::CPlayerControllerComponent::OnDrawGUI()
{
    return false;
}

bool KG::Component::CPlayerControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
    return false;
}

void KG::Component::CPlayerControllerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CPlayerControllerComponentSystem::OnPreRender()
{
}
