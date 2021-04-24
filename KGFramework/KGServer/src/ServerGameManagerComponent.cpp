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
	if (this->server->isConnect) {
		if (timer < 1.0f)
			timer += elapsedTime;
		else {
			KG::Packet::SC_SCENE_DATA DataPacket = {};
			DataPacket.position = KG::Packet::RawFloat3(rand() % 5, 0, rand() % 5);
			// DataPacket.positions = this->server->positions
			this->BroadcastPacket(&DataPacket);
			std::cout << "send scene data\n";
			timer = 0;
		}
	}
	/*
	����
	------
	���� ���� �÷��̾� �߰� �� �ش� �÷��̾��� ���� ����(Ʈ������, �ִϸ��̼� ��)
	���� ���� �÷��̾� �߰� �� �ش� �÷��̾��� ��ǲ ���� ����->Ŭ�󿡼� ��ǲ ���� ���� ������ ���ŵǴ� ����
	������Ʈ���� ��ǲ ������ ���� �̵� �� �ִϸ��̼� ���� ���� + ��ε�ĳ�������� ��� �÷��̾� ����(���� ���� ���� ���� ����) ����
	
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
		std::cout << "Error Packet Received\n";
		return false;
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		auto id = this->server->GetNewObjectId();

		//�÷��̾� �߰�!
		this->server->LockWorld();
		this->server->isConnect = true;
		auto* playerComp = static_cast<KG::Component::SBaseComponent*>(this->gameObject->GetScene()->CallNetworkCreator("TeamCharacter"_id));
		playerComp->SetNetObjectId(id);
		auto* trans = playerComp->GetGameObject()->GetTransform();
		trans->SetPosition(id, 0, id);
		this->GetGameObject()->GetTransform()->AddChild(trans);
		this->server->UnlockWorld();
		//this->server->Addplayer(trans)

		KG::Packet::SC_PLAYER_INIT initPacket = {};
		initPacket.playerObjectId = id;
		initPacket.position = KG::Packet::RawFloat3(id, 0, id);
		initPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		this->SendPacket(sender, &initPacket);

		KG::Packet::SC_ADD_PLAYER addPacket = {};
		addPacket.playerObjectId = id;
		addPacket.position = KG::Packet::RawFloat3(id, 0, id);
		addPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		this->BroadcastPacket(&addPacket, sender);
	}
	return true;
	case KG::Packet::PacketType::CS_INPUT: {
		// this->server->inputs
		// �ش� ��ǲ�� ���� Ŭ���̾�Ʈ�� ��ǲ ���� ����
	}
										 return true;
	case KG::Packet::PacketType::CS_FIRE:
		std::cout << "Error Packet Received\n";
		return false;
	}
	return false;
}

void KG::Component::SGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::SGameManagerComponentSystem::OnPreRender()
{
}
