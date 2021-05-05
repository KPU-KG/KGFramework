#include "pch.h"
#include "ServerGameManagerComponent.h"
#include "Scene.h"
#include "KGServer.h"
#include "Transform.h"
#include "PhysicsComponent.h"
#include "IPhysicsScene.h"
#include <random>

std::random_device rdRegion;
std::mt19937 genRegion(rdRegion());
std::uniform_int_distribution<int> randomRegion(0, MAX_REGION - 1);

bool KG::Component::EnemyGenerator::OnDrawGUI(KG::Core::GameObject* gameObject)
{
	if (ImGui::ComponentHeader<KG::Component::SEnemyControllerComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		auto view = gameObject->GetScene()->GetMainCameraView();
		auto proj = gameObject->GetScene()->GetMainCameraProj();

		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);

		for (int i = 0; i < MAX_REGION; ++i) {
			std::string s("REGION" + std::to_string(i));
			ImGui::TextDisabled(s.c_str());
			std::string ps(s + " Center");
			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(ps, this->region[i].position);
			std::string rs(s + " Range");
			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->region[i].range);

			DirectX::XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat,
				DirectX::XMMatrixScaling(this->region[i].range * 2, 0.1, this->region[i].range * 2) *
				DirectX::XMMatrixTranslation(this->region[i].position.x, this->region[i].position.y, this->region[i].position.z)
			);

			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}
	}
	return false;
}

bool KG::Component::EnemyGenerator::IsGeneratable() const
{
	bool allDead = true;
	for (auto& enemy : enemies) {
		if (!enemy->IsDead()) {
			allDead = false;
			break;
		}
	}
	return allDead;
}

KG::Component::Region KG::Component::EnemyGenerator::GetNextRegion()
{
	currentRegion = randomRegion(genRegion);
	return region[currentRegion];
}

void KG::Component::EnemyGenerator::AddEnemyControllerCompoenent(KG::Component::SEnemyControllerComponent* comp)
{
	enemies.emplace_back(comp);
}

int KG::Component::EnemyGenerator::GetCurrentRegionIndex() const
{
	return currentRegion;
}

KG::Component::Region KG::Component::EnemyGenerator::GetCurrentRegion()
{
	return this->region[currentRegion];
}

void KG::Component::SGameManagerComponent::GenerateEnemy()
{
	auto region = enemyGenerator.GetNextRegion();

	std::uniform_real_distribution<float> randomPos(-region.range, region.range);
	auto presetName = "EnemyMech";
	auto presetId = KG::Utill::HashString(presetName);

	auto* scene = this->gameObject->GetScene();
	auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

	DirectX::XMFLOAT3 genPos{
		randomPos(genRegion) + region.position.x,
		region.position.y,
		randomPos(genRegion) + region.position.z
	};

	KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
	auto tag = KG::Utill::HashString(presetName);
	addObjectPacket.objectTag = tag;
	addObjectPacket.parentTag = 0;
	addObjectPacket.presetId = tag;
	addObjectPacket.position = genPos;

	auto id = this->server->GetNewObjectId();
	addObjectPacket.newObjectId = id;
	comp->SetNetObjectId(id);
	this->server->SetServerObject(id, comp);

	auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyControllerComponent>();
	enemyCtrl->SetCenter(region.position);
	enemyCtrl->SetWanderRange(region.range);
	enemyCtrl->SetPosition(genPos);

	enemyGenerator.AddEnemyControllerCompoenent(enemyCtrl);

	this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
	this->BroadcastPacket(&addObjectPacket);
}

void KG::Component::SGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
	this->server->SetServerObject(this->networkObjectId, this);
	this->physicsScene = this->server->GetPhysicsScene();
}

void KG::Component::SGameManagerComponent::Update(float elapsedTime)
{
	if (enemyGenerator.IsGeneratable()) {
		if (generateEnemy)
			GenerateEnemy();
	}
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
				comp->SetNetObjectId(id);
				this->server->SetServerObject(id, comp);
				this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
			}

			this->BroadcastPacket(&addObjectPacket);
		}
		enemyGenerator.OnDrawGUI(this->gameObject);
		std::string s{"Generate Enemy"};
		KG::Utill::ImguiProperty::DrawGUIProperty<bool>(s, this->generateEnemy);
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
		auto newPlayerId = this->server->GetNewObjectId();

		//플레이어 추가!
		this->server->LockWorld();
		this->server->SetSessionState(sender, KG::Server::PLAYER_STATE::PLAYER_STATE_INGAME);
		auto* playerComp = static_cast<KG::Component::SPlayerComponent*>(this->gameObject->GetScene()->CallNetworkCreator("TeamCharacter"_id));
		auto* dyn = playerComp->GetGameObject()->GetComponent<DynamicRigidComponent>();
		playerComp->SetNetObjectId(newPlayerId);
		this->SetPhysicsScene(this->physicsScene);
		auto* trans = playerComp->GetGameObject()->GetTransform();
		this->GetGameObject()->GetTransform()->AddChild(trans);
		trans->SetPosition(newPlayerId + 10, 1, newPlayerId + 10);
		dyn->SetPosition(XMFLOAT3(newPlayerId + 10, 1, newPlayerId + 10));
		playerObjects.insert(std::make_pair(newPlayerId, playerComp));
		this->server->SetServerObject(newPlayerId, playerComp);
		this->server->UnlockWorld();

		KG::Packet::SC_PLAYER_INIT initPacket = {};
		initPacket.playerObjectId = newPlayerId;
		initPacket.position = KG::Packet::RawFloat3(newPlayerId + 10, 1, newPlayerId + 5);
		initPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		this->SendPacket(sender, &initPacket);

		KG::Packet::SC_ADD_PLAYER addPacket = {};
		addPacket.playerObjectId = newPlayerId;
		addPacket.position = KG::Packet::RawFloat3(newPlayerId + 10, 1, newPlayerId + 5);
		addPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		this->BroadcastPacket(&addPacket, sender);

		for ( auto& [id, ptr] : this->playerObjects )
		{
			if ( id == newPlayerId ) continue;
			std::shared_lock sl{ ptr->playerInfoLock };

			KG::Packet::SC_ADD_PLAYER addPacket = {};
			addPacket.playerObjectId = id;
			addPacket.position = ptr->GetGameObject()->GetTransform()->GetPosition();
			addPacket.rotation = ptr->GetGameObject()->GetTransform()->GetRotation();
			this->SendPacket(sender, &addPacket);
		}
	}
	return true;
	}
	return false;
}

