#include "pch.h"
#include "ServerPlayerControllerComponent.h"

void KG::Component::SPlayerComponent::OnCreate(KG::Core::GameObject* obj)
{
}

void KG::Component::SPlayerComponent::Update(float elapsedTime)
{
}

bool KG::Component::SPlayerComponent::OnDrawGUI()
{
	return false;
}

bool KG::Component::SPlayerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

void KG::Component::SPlayerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::SPlayerComponentSystem::OnPreRender()
{
}
