#include "pch.h"
#include "ServerGameManagerComponent.h"
#include "Scene.h"
#include "KGServer.h"
#include "Transform.h"

void KG::Component::SGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
	this->server->SetServerObject(this->networkObjectId, this);
}

void KG::Component::SGameManagerComponent::Update(float elapsedTime)
{
	DebugNormalMessage("Server Update");
	
	/*
	서버
	------
	벡터 만들어서 플레이어 추가 시 해당 플레이어의 정보 저장(트랜스폼, 애니메이션 등)
	벡터 만들어서 플레이어 추가 시 해당 플레이어의 인풋 정보 저장->클라에서 인풋 정보 보낼 때마다 갱신되는 정보
	업데이트에서 인풋 정보에 따라 이동 및 애니메이션 정보 갱신 + 브로드캐스팅으로 모든 플레이어 정보(위에 만든 벡터 내부 정보) 전송

	*/


}

bool KG::Component::SGameManagerComponent::OnDrawGUI()
{
	static char presetName[256] = {};
	if (ImGui::ComponentHeader<SGameManagerComponent>())
	{
		ImGui::InputText("presetName", presetName, 256);
		ImGui::SameLine();
		if (ImGui::Button("Add Server"))
		{
			auto presetId = KG::Utill::HashString(presetName);
			auto* scene = this->gameObject->GetScene();
			auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(presetId));

			KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
			auto tag = KG::Utill::HashString(presetName);
			addObjectPacket.objectTag = tag;
			addObjectPacket.parentTag = 0;
			addObjectPacket.presetId = tag;
			addObjectPacket.position = KG::Packet::RawFloat3();

			if (!comp)
			{
				addObjectPacket.newObjectId = KG::Server::NULL_NET_OBJECT_ID;

				auto* obj = scene->CallPreset(presetId);
				this->GetGameObject()->GetTransform()->AddChild(obj->GetTransform());
			}
			else
			{
				auto id = this->server->GetNewObjectId();
				addObjectPacket.newObjectId = id;
				this->server->SetServerObject(id, comp);
				this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
			}

			this->BroadcastPacket(&addObjectPacket);
		}
	}
	return false;
}

bool KG::Component::SGameManagerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	switch (type)
	{
	case KG::Packet::PacketType::None:
	case KG::Packet::PacketType::PacketHeader:
	case KG::Packet::PacketType::SC_LOGIN_OK:
	case KG::Packet::PacketType::SC_PLAYER_INIT:
	case KG::Packet::PacketType::SC_ADD_OBJECT:
	case KG::Packet::PacketType::SC_REMOVE_OBJECT:
	case KG::Packet::PacketType::SC_FIRE:
	case KG::Packet::PacketType::SC_ADD_PLAYER:
	case KG::Packet::PacketType::SC_PLAYER_SYNC:
	case KG::Packet::PacketType::CS_INPUT:
	case KG::Packet::PacketType::CS_FIRE:
		std::cout << "Error Packet Received\n";
		return false;
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		auto id = this->server->GetNewObjectId();

		//플레이어 추가!
		this->server->LockWorld();

		auto* playerComp = static_cast<KG::Component::SPlayerComponent*>(this->gameObject->GetScene()->CallNetworkCreator("TeamCharacter"_id));
		playerComp->SetNetObjectId(id);
		auto* trans = playerComp->GetGameObject()->GetTransform();
		trans->SetPosition(id, 0, id);
		this->GetGameObject()->GetTransform()->AddChild(trans);
		this->server->UnlockWorld();

		playerComp->InitData(playerObjectIds, positions);

		for (size_t i = 0; i < 4; i++)
		{
			if (playerObjectIds[i] == 0) {
				playerObjectIds[i] = id;
				this->server->SetServerObject(id, playerComp);

				KG::Packet::SC_PLAYER_INIT initPacket = {};
				initPacket.playerObjectId = id;
				initPacket.position = KG::Packet::RawFloat3(id, 0, id);
				initPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
				this->SendPacket(sender, &initPacket);

				for (size_t i = 0; i < 4; i++)
				{
					if (playerObjectIds[i] != 0 && playerObjectIds[i] != id) {
						KG::Packet::SC_ADD_PLAYER addPacket = {};
						addPacket.playerObjectId = playerObjectIds[i];
						addPacket.position = KG::Packet::RawFloat3(addPacket.playerObjectId, 0, addPacket.playerObjectId);
						addPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
						this->SendPacket(sender, &addPacket);
					}
				} // 앞서 접속된 플레이어 있을 경우 추가 패킷 전송 

				KG::Packet::SC_ADD_PLAYER addPacket = {};
				addPacket.playerObjectId = id;
				addPacket.position = KG::Packet::RawFloat3(id, 0, id);
				addPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
				this->BroadcastPacket(&addPacket, sender);
				break;
			}
		}
		
		
	}
	return true;
	}
	return false;
}

void KG::Component::SGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::SGameManagerComponentSystem::OnPreRender()
{
}
