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
	//char a = (char)this->network->GetLobbyInfo(0);
	//char b = (char)this->network->GetLobbyInfo(1);
	//std::cout << a << b << std::endl;
}

bool KG::Component::CGameManagerComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<CGameManagerComponent>())
	{
		if (ImGui::Button("REQ_LOGIN"))
		{
            this->SendLoginPacket();
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
			if ( addPacket->newObjectId == KG::Server::NULL_NET_OBJECT_ID )
			{
				auto* obj = this->gameObject->GetScene()->CallPreset(addPacket->presetId);
				this->GetGameObject()->GetTransform()->AddChild(obj->GetTransform());
			}
			else
			{
				auto* newComp = static_cast<KG::Component::CBaseComponent*>(this->gameObject->GetScene()->CallNetworkCreator(addPacket->presetId));
				newComp->SetNetObjectId(addPacket->newObjectId);
				this->network->SetNetworkObject(addPacket->newObjectId, (newComp));
				this->GetGameObject()->GetTransform()->AddChild(newComp->GetGameObject()->GetTransform());
			}
		}
		return true;
		case KG::Packet::PacketType::SC_PLAYER_INIT:
		{
			auto* initPacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_INIT>(packet);
			auto* playerController = static_cast<KG::Component::CBaseComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("PlayerCharacter"_id));
			playerController->SetNetObjectId(initPacket->playerObjectId);
			this->network->SetNetworkObject(initPacket->playerObjectId, playerController);
			auto* playerTransform = playerController->GetGameObject()->GetTransform();
			playerTransform->SetPosition(initPacket->position);
			this->GetGameObject()->GetTransform()->AddChild(playerTransform);
		}
		return true;
		case KG::Packet::PacketType::SC_ADD_PLAYER:
		{
			auto* addPlayerPacket = KG::Packet::PacketCast<KG::Packet::SC_ADD_PLAYER>(packet);
			auto* teamController = static_cast<KG::Component::CBaseComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("TeamCharacter"_id));
			teamController->SetNetObjectId(addPlayerPacket->playerObjectId);
			this->network->SetNetworkObject(addPlayerPacket->playerObjectId, teamController);
			auto* trans = teamController->GetGameObject()->GetTransform();
			trans->SetPosition(addPlayerPacket->position);
			this->GetGameObject()->GetTransform()->AddChild(trans);
		}
		return true;
		case KG::Packet::PacketType::SC_SCENE_DATA:
		{
			//if (playerController != nullptr) {
			//	auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_SCENE_DATA>(packet);
			//	std::cout << "scene data recv" << std::endl;
			//	playerTransform->SetPosition(ScenePacket->position);
			//}
		}
		return true;
		case KG::Packet::PacketType::SC_GAME_END:
		{
			// 게임 종료 패킷 수신 -> 씬초기화 + 로비 재접속?
		}
		return true;
	}
	return false;
}

void KG::Component::CGameManagerComponent::SendLoginPacket()
{
    KG::Packet::CS_REQ_LOGIN login = {};
    this->SendPacket(&login);
}
