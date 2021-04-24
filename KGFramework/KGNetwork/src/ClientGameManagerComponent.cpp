#include "pch.h"
#include "Protocol.h"
#include "Network.h"
#include "ClientGameManagerComponent.h"
#include "InputManager.h"
#include "Scene.h"
#include "Transform.h"

void KG::Component::CGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
	this->network->SetNetworkObject(this->networkObjectId, this);
}

void KG::Component::CGameManagerComponent::Update(float elapsedTime)
{
	if (playerController != nullptr) {
		KG::Packet::CS_INPUT inputPacket = {};
		auto input = KG::Input::InputManager::GetInputManager();
		if (input->IsTouching('W'))
			inputPacket.stateW = true;
		else
			inputPacket.stateW = false;
		if (input->IsTouching('S'))
			inputPacket.stateS = true;
		else
			inputPacket.stateS = false;
		if (input->IsTouching('D'))
			inputPacket.stateD = true;
		else
			inputPacket.stateD = false;
		if (input->IsTouching('A'))
			inputPacket.stateA = true;
		else
			inputPacket.stateA = false;
		inputPacket.stateShift = false;
		if (updatetimer < 1.0f)
			updatetimer += elapsedTime;
		else {
			playerController->SendPacket(&inputPacket);
			updatetimer = 0;
		}
	}
	
}

bool KG::Component::CGameManagerComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<CGameManagerComponent>() )
	{
		if ( ImGui::Button("REQ_LOGIN") )
		{
			KG::Packet::CS_REQ_LOGIN login = {};
			this->SendPacket(&login);
		}
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
			if ( addPacket->newObjectId == KG::Server::NULL_NET_OBJECT_ID )
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
			playerController = static_cast<KG::Component::CBaseComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("PlayerCharacter"_id));
			playerController->SetNetObjectId(initPacket->playerObjectId);
			this->network->SetNetworkObject(initPacket->playerObjectId, playerController);
			playerTransform = playerController->GetGameObject()->GetTransform();
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
			/*teamControllers.emplace_back(teamController);
			teamTransforms.emplace_back(trans)*/
		}
		return true;
		case KG::Packet::PacketType::SC_PLAYER_DATA:
		{
			if (playerController != nullptr) {
				auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_DATA>(packet);

				for (size_t i = 0; i < 4; i++)
				{
					if (ScenePacket->playerObjectIds[i] != KG::Server::NULL_NET_OBJECT_ID) {
						if (ScenePacket->playerObjectIds[i] == networkObjectId) {
							std::cout << "scene data recv" << std::endl;
							playerTransform->SetPosition(ScenePacket->positions[i]);
						}
						else {
							/*auto* teamController = static_cast<KG::Component::CBaseComponent*>(this->GetGameObject()->GetScene()->CallNetworkCreator("TeamCharacter"_id));
							auto* trans = teamController->GetGameObject()->GetTransform();
							trans->SetPosition(ScenePacket->positions[i]);*/
						}
					}
				}
			}
		}
		return true;
		
	/*	클라
		------
		플레이어 정보 받고 플레이어 + 팀 정보로 처리
		인풋 전송
		데드레커닝
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
