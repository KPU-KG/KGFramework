#include "pch.h"
#include "Protocol.h"
#include "Network.h"
#include "ClientGameManagerComponent.h"
#include "Scene.h"

void KG::Component::CGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
	this->network->SetNetworkObject(this->networkObjectId, this);
}

void KG::Component::CGameManagerComponent::Update(float elapsedTime)
{

}

bool KG::Component::CGameManagerComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<CGameManagerComponent>() )
	{
	}
	return false;
}

bool KG::Component::CGameManagerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch ( type )
	{
		case KG::Packet::PacketType::SC_ADD_OBJECT:
		{
			auto* addPacket = KG::Packet::PacketCast<KG::Packet::SC_ADD_OBJECT>(packet);
			auto* newComp = static_cast<KG::Component::CBaseComponent*>(this->gameObject->GetScene()->CallNetworkCreator(addPacket->newObjectId));
			newComp->SetNetObjectId(addPacket->newObjectId);
			this->network->SetNetworkObject(addPacket->newObjectId, (newComp));
		}
		return true;
	}
	return false;
}

void KG::Component::CGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CGameManagerComponentSystem::OnPreRender()
{
}
