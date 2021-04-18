#include "pch.h"
#include "ServerGameManagerComponent.h"
#include "Scene.h"
#include "KGServer.h"
#include "Transform.h"

void KG::Component::SGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
}

void KG::Component::SGameManagerComponent::Update(float elapsedTime)
{
}

bool KG::Component::SGameManagerComponent::OnDrawGUI()
{
	static char presetName[256] = {};
	if ( ImGui::ComponentHeader<SGameManagerComponent>() )
	{
		ImGui::InputText("presetName", presetName, 256);
		ImGui::SameLine();
		if ( ImGui::Button("Add Server") )
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

			if ( !comp )
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

void KG::Component::SGameManagerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::SGameManagerComponentSystem::OnPreRender()
{
}
