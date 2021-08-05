#include "pch.h"
#include "ServerGameManagerComponent.h"
#include "Scene.h"
#include "KGServer.h"
#include "Transform.h"
#include "PhysicsComponent.h"
#include "IPhysicsScene.h"
#include <random>
#include "ServerEnemyMechComponent.h"

std::random_device rdRegion;
std::mt19937 genRegion(rdRegion());
std::uniform_int_distribution<int> randomSpawn(3, 4);
std::uniform_int_distribution<int> enemyType(0, 1);

KG::Component::Region::Region()
	:
	positionProp("RegionPosition", position),
	rangeProp("RegionRange", range),
	heightOffsetProp("RegionHeightOffset", heightOffset)
{

}

KG::Component::Region::Region(DirectX::XMFLOAT3 position, float range, float heightOffset) : Region() 
{
	this->position = position;
	this->range = range;
	this->heightOffset = heightOffset;
}
KG::Component::Region::Region(const KG::Component::Region& other) : Region()
{
	this->position = other.position;
	this->range = other.range;
	this->heightOffset = other.heightOffset;
}
KG::Component::Region::Region(KG::Component::Region&& other) : Region()
{
	this->position = other.position;
	this->range = other.range;
	this->heightOffset = other.heightOffset;
}

KG::Component::Region& KG::Component::Region::operator=(const KG::Component::Region& other) {
	this->position = other.position;
	this->range = other.range;
	this->heightOffset = other.heightOffset;
	return *this;
}

KG::Component::Region& KG::Component::Region::operator=(KG::Component::Region&& other) {
	this->position = other.position;
	this->range = other.range;
	this->heightOffset = other.heightOffset;
	return *this;
}

bool KG::Component::EnemyGeneratorComponent::OnDrawGUI()
{
	ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

	for (int i = 0; i < this->region.size(); ++i) {
		std::string s("REGION" + std::to_string(i));
		ImGui::TextDisabled(s.c_str());
		std::string ps(s + " Center");
		KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(ps, this->region[i].position);
		std::string rs(s + " Range");
		KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->region[i].range);
		std::string hs(s + " Height Offset");
		KG::Utill::ImguiProperty::DrawGUIProperty<float>(hs, this->region[i].heightOffset);

		std::string bn("Delete Region " + std::to_string(i));

		if (ImGui::Button(bn.c_str()))
		{
			this->region.erase(this->region.begin() + i);
			return false;
		}
	}

	if (ImGui::Button("Add Region"))
	{
		this->region.emplace_back();
	}

	if (gameObject != nullptr && gameObject->GetScene()->GetMainCamera() != nullptr) {
		auto view = gameObject->GetScene()->GetMainCameraView();
		auto proj = gameObject->GetScene()->GetMainCameraProj();

		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);

		for (auto& r : this->region) {
			DirectX::XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat,
				DirectX::XMMatrixScaling(r.range * 2, 0.1, r.range * 2) *
				DirectX::XMMatrixTranslation(r.position.x, r.position.y, r.position.z)
			);

			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}

		std::string s{ "Generate Enemy" };
		KG::Utill::ImguiProperty::DrawGUIProperty<bool>(s, this->generateEnemy);
	}
	return false;
}

void KG::Component::EnemyGeneratorComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	for (int x = 0; x < MAP_SIZE_X; ++x) {
		std::fill(session[x].begin(), session[x].end(), SESSION_TYPE_NONE);
	}
}

void KG::Component::EnemyGeneratorComponent::Update(float elapsedTime)
{
	bool destroyFlag = false;
	while (!destroyFlag) {
		destroyFlag = true;
		for (int i = 0; i < enemies.size(); ++i) {
			if (enemies[i]->IsDelete()) {
				enemies[i]->Destroy();
				enemies.erase(enemies.begin() + i);
				destroyFlag = false;
				// 적 종류에 따라 점수 증가
				score += 10;
				break;
			}
		}
	}

	for (auto& e : this->enemies) {
		if (e->IsAttackable()) {
			this->isAttackable = true;
			break;
		}
	}
}

KG::Component::EnemyGeneratorComponent::EnemyGeneratorComponent()
	:
	generateProp("GenerateEnemy", generateEnemy)
{
	session.resize(MAP_SIZE_X);
	for (int i = 0; i < session.size(); ++i)
		session[i].resize(MAP_SIZE_Z);
}

KG::Component::EnemyGeneratorComponent::~EnemyGeneratorComponent()
{
	for (int i = 0; i < session.size(); ++i)
		session[i].clear();
	session.clear();

}

void KG::Component::EnemyGeneratorComponent::Initialize()
{
	if (initialized)
		return;

	auto physicsScene = this->server->GetPhysicsScene();
	if (physicsScene == nullptr)
		return;
	if (!physicsScene->IsInitialized())
		return;

	auto actorExtents = physicsScene->GetStaticActorExtents();
	for (auto extent : actorExtents) {
		auto bound = extent.first;
		auto center = extent.second;
		int bx = bound.first;
		int bz = bound.second;
		int cx = center.first + (MAP_SIZE_X / 2);
		int cz = center.second + (MAP_SIZE_Z / 2);

		int x = max(0, cx - bx);
		int w = min(MAP_SIZE_X, cx + bx);
		int z = max(0, cz - bz);
		int d = min(MAP_SIZE_Z, cz + bz) - z;
		if (z + d >= MAP_SIZE_Z)
			d = MAP_SIZE_Z - z;

		if (w - x > 200 || d > 200)
			continue;
		for (; x < w; ++x) {
			memset(&session[x][z], SESSION_TYPE_BUILDING, d);
		}

#ifdef _DEBUG
		DebugNormalMessage("bound : " << bx << ", " << bz << " / center : " << cx << ", " << cz);
#endif
	}

#ifdef _DEBUG
	DebugNormalMessage("Enemy Generator : Initialize Session");
#endif

	initialized = true;
}

bool KG::Component::EnemyGeneratorComponent::IsGeneratable() const
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

KG::Component::Region KG::Component::EnemyGeneratorComponent::GetNextRegion()
{
	std::uniform_int_distribution<int> randomRegion(0, this->region.size() - 1);
	currentRegion = randomRegion(genRegion);
	return region[3];
	// return region[currentRegion];
}

void KG::Component::EnemyGeneratorComponent::AddEnemyControllerCompoenent(KG::Component::SEnemyUnitComponent* comp)
{
	enemies.emplace_back(comp);
}

int KG::Component::EnemyGeneratorComponent::GetCurrentRegionIndex() const
{
	return currentRegion;
}

KG::Component::Region KG::Component::EnemyGeneratorComponent::GetCurrentRegion()
{
	return this->region[currentRegion];
}

void  KG::Component::EnemyGeneratorComponent::SendAddEnemyPacket(KG::Server::SESSION_ID player) {
	for (auto& e : this->enemies) {
		auto presetName = e->GetEnemyPresetName();						// 적 종류 추가되면 enemy에서 불러올 것
		auto presetId = KG::Utill::HashString(presetName);

		auto* scene = this->gameObject->GetScene();

		auto t = e->GetGameObject()->GetTransform();

		KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
		auto tag = KG::Utill::HashString(presetName);
		addObjectPacket.objectTag = tag;
		addObjectPacket.parentTag = 0;
		addObjectPacket.presetId = tag;
		addObjectPacket.position = t->GetPosition();

		auto id = e->GetNetId();
		addObjectPacket.newObjectId = id;
		this->SendPacket(player, (void*)&addObjectPacket);
	}
}

unsigned int KG::Component::EnemyGeneratorComponent::GetScore() const {
	return this->score;
}

void KG::Component::EnemyGeneratorComponent::OnDataLoad(tinyxml2::XMLElement* objectElement) {
	this->region.clear();
	generateProp.OnDataLoad(objectElement);
	auto* nextElement = objectElement->FirstChildElement("Region");
	while (nextElement != nullptr)
	{
		Region r;
		r.positionProp.OnDataLoad(nextElement);
		r.rangeProp.OnDataLoad(nextElement);
		this->region.emplace_back(r);
		nextElement = nextElement->NextSiblingElement("Region");
	}
}

void KG::Component::EnemyGeneratorComponent::OnDataSave(tinyxml2::XMLElement* objectElement) {
	auto* componentElement = objectElement->InsertNewChildElement("Component");
 	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::EnemyGeneratorComponent);
	generateProp.OnDataSave(componentElement);
	for (auto& i : this->region)
	{
		auto* element = componentElement->InsertNewChildElement("Region");
		i.positionProp.OnDataSave(element);
		i.rangeProp.OnDataSave(element);
	}
}

void KG::Component::EnemyGeneratorComponent::RegisterPlayerToEnemy(KG::Server::NET_OBJECT_ID id)
{
	for (auto& e : this->enemies) {
		e->RegisterPlayerId(id);
	}
}

void KG::Component::EnemyGeneratorComponent::DeregisterPlayerToEnemy(KG::Server::NET_OBJECT_ID id)
{
	for (auto& e : this->enemies) {
		e->DeregisterPlayerId(id);
	}
}

void KG::Component::EnemyGeneratorComponent::SendAttackPacket(SGameManagerComponent* gameManager)
{
	for (auto& e : this->enemies) {
		if (e->IsAttackable()) {
			e->Attack(gameManager);
			e->PostAttack();
		}
	}
	this->isAttackable = false;
}

void KG::Component::EnemyGeneratorComponent::GenerateEnemy()
{
	if (this->region.size() == 0)
		return;
	if (!this->generateEnemy)
		return;
	auto region = GetNextRegion();

	int enemyCount = 1; // randomSpawn(genRegion);

	for (int i = 0; i < enemyCount; ++i) {
		std::uniform_real_distribution<float> randomPos(-region.range, region.range);
		int type = enemyType(genRegion);
		// const char* presetName = nullptr;
		// switch (type) {
		// case 0:
		// 	presetName = "EnemyMech";
		// 	break;
		// case 1:
		// 	presetName = "EnemyCrawler";
		// 	break;
		// }
		// auto presetName = "EnemyCrawler";
		auto presetName = "EnemyMech";

		auto presetId = KG::Utill::HashString(presetName);

		auto* scene = this->gameObject->GetScene();
		auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

		DirectX::XMFLOAT3 genPos{
			randomPos(genRegion) + region.position.x,
			region.position.y + region.heightOffset,
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

		// switch (type) {
		// case 0:
		// {
		// 
		// 	auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyMechComponent>();
		// 	enemyCtrl->SetCenter(region.position);
		// 	enemyCtrl->SetWanderRange(region.range);
		// 	enemyCtrl->SetPosition(genPos);
		// 	AddEnemyControllerCompoenent(enemyCtrl);
		// }
		// 	break;
		// case 1:
		// {
		// 
		// 	auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyCrawlerComponent>();
		// 	enemyCtrl->SetCenter(region.position);
		// 	enemyCtrl->SetWanderRange(region.range);
		// 	enemyCtrl->SetPosition(genPos);
		// 	AddEnemyControllerCompoenent(enemyCtrl);
		// }
		// 	break;
		// }

		// auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyCrawlerComponent>();
		auto enemyCtrl = comp->GetGameObject()->GetComponent<SEnemyMechComponent>();
		enemyCtrl->SetCenter(region.position);
		enemyCtrl->SetWanderRange(region.range);
		enemyCtrl->SetPosition(genPos);
		AddEnemyControllerCompoenent(enemyCtrl);
		enemyCtrl->SetEnemyPresetName(presetName);
		enemyCtrl->SetSession(session);
		this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
		this->BroadcastPacket(&addObjectPacket);
	}
}

void  KG::Component::SGameManagerComponent::RegisterPlayersToEnemy() {
	for (auto& id : playerObjects) {
		enemyGenerator->RegisterPlayerToEnemy(id.first);
	}
}

void KG::Component::SGameManagerComponent::UpdatePlayerSession() {
	auto& pid = this->server->GetDisconnectedPlayerId();
	while (!pid.empty()) {
		auto id = pid.top();
		if (playerObjects.count(id) > 0) {
			playerObjects.unsafe_erase(id);
			enemyGenerator->DeregisterPlayerToEnemy(id);
		}
		pid.pop();
	}
}

void KG::Component::SGameManagerComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->SetNetObjectId(KG::Server::SCENE_CONTROLLER_ID);
	this->server->SetServerObject(this->networkObjectId, this);
	this->physicsScene = this->server->GetPhysicsScene();
}

void KG::Component::SGameManagerComponent::Update(float elapsedTime)
{
	this->UpdatePlayerSession();
	if (enemyGenerator == nullptr) {
		enemyGenerator = this->gameObject->GetComponent<EnemyGeneratorComponent>();
		if (enemyGenerator) {
			enemyGenerator->SetServerInstance(this->server);
			enemyGenerator->SetNetObjectId(this->networkObjectId);
		}
	}

	if (enemyGenerator) {
		if (enemyGenerator->IsGeneratable()) {
			enemyGenerator->Initialize();
			enemyGenerator->GenerateEnemy();
			for (auto& p : playerObjects) {
				p.second->playerInfoLock.lock();
				enemyGenerator->RegisterPlayerToEnemy(p.first);
				p.second->playerInfoLock.unlock();
			}
		}
		if (enemyGenerator->isAttackable) {
			enemyGenerator->SendAttackPacket(this);
		}
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
	}
	return false;
}

// 플레이어 등록해줘야 할 곳
// 1. 플레이어 req 시점
// 2. enemy 추가 시점

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
		this->server->SetSessionId(sender, newPlayerId);
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

		// if (enemyGenerator) {
		// 	enemyGenerator->RegisterPlayerToEnemy(newPlayerId);
		// }

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

		RegisterPlayersToEnemy();

		// 이미 생성되어있는 적 추가
		if (this->enemyGenerator)
			this->enemyGenerator->SendAddEnemyPacket(sender);
	}
	return true;
	}
	return false;
}

