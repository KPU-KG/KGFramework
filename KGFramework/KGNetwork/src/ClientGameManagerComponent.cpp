#include "pch.h"
#include "Protocol.h"
#include "Network.h"
#include "ClientGameManagerComponent.h"

#include "Scene.h"
#include "Transform.h"

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
	if (ImGui::ComponentHeader<CGameManagerComponent>())
	{
		if (ImGui::Button("REQ_LOGIN"))
		{
			KG::Packet::CS_REQ_LOGIN login = {};
			this->SendPacket(&login);
		}
	}
	return false;
}

bool KG::Component::CGameManagerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{

	switch (type)
	{
	case KG::Packet::PacketType::SC_ADD_OBJECT:
	{
		auto* addPacket = KG::Packet::PacketCast<KG::Packet::SC_ADD_OBJECT>(packet);
		if (addPacket->newObjectId == KG::Server::NULL_NET_OBJECT_ID)
		{
			auto* obj = this->gameObject->GetScene()->CallPreset(addPacket->presetId);
			this->GetGameObject()->GetTransform()->AddChild(obj->GetTransform());
		}
		else
		{
			auto* newComp = static_cast<KG::Component::CBaseComponent*>(this->gameObject->GetScene()->CallNetworkCreator(addPacket->newObjectId));
			newComp->SetNetObjectId(addPacket->newObjectId);
			this->network->SetNetworkObject(addPacket->newObjectId, (newComp));
			this->GetGameObject()->GetTransform()->AddChild(newComp->GetGameObject()->GetTransform());
		}
	}
	return true;
	case KG::Packet::PacketType::SC_PLAYER_INIT:
	{
		auto* initPacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_INIT>(packet);
		playerController = static_cast<KG::Component::CPlayerControllerComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("PlayerCharacter"_id));
		playerController->SetNetObjectId(initPacket->playerObjectId);
		playerController->id = initPacket->playerObjectId;
		this->network->SetNetworkObject(initPacket->playerObjectId, playerController);
		playerController->transform = playerController->GetGameObject()->GetTransform();
		playerController->transform->SetPosition(initPacket->position);
		this->GetGameObject()->GetTransform()->AddChild(playerController->transform);
	}
	return true;
	case KG::Packet::PacketType::SC_ADD_PLAYER:
	{
		auto* addPlayerPacket = KG::Packet::PacketCast<KG::Packet::SC_ADD_PLAYER>(packet);
		for (size_t i = 0; i < 4; i++)
		{
			if (teamControllers[i] == nullptr) {
				teamControllers[i] = static_cast<KG::Component::CPlayerControllerComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("TeamCharacter"_id));
				teamControllers[i]->SetNetObjectId(addPlayerPacket->playerObjectId);
				teamControllers[i]->id = addPlayerPacket->playerObjectId;
				this->network->SetNetworkObject(addPlayerPacket->playerObjectId, teamControllers[i]);
				teamControllers[i]->transform = teamControllers[i]->GetGameObject()->GetTransform();
				teamControllers[i]->transform->SetPosition(addPlayerPacket->position);
				this->GetGameObject()->GetTransform()->AddChild(teamControllers[i]->transform);
				break;
			}
		}
	}
	return true;

	/*	클라
		------
		플레이어 정보 받고 플레이어 + 팀 정보로 처리
		인풋 전송
		데드레커닝 - kgmain->playercontroller 계산과 동일하게 서버 연산 +
	*/
	}
	return false;
}

void KG::Component::CGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CGameManagerComponentSystem::OnPreRender()
{
}
