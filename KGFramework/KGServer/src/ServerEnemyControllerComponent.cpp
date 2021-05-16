#include "pch.h"
#include "ServerEnemyControllerComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IAnimationComponent.h"

#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include "Scene.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> goalRange(-0.5, 0.5);

void KG::Component::SEnemyControllerComponent::UpdateState()
{

}

bool KG::Component::SEnemyControllerComponent::SetGoal()
{
	switch (state) {
	case EnemyState::eWANDER:
	{
		if (this->nodeCount > 0) {
			if (randomCircuit) {
				std::uniform_int_distribution<int> randomIndex(1, this->nodeCount);
				currentNode = randomIndex(gen) - 1;
			}
			else {
				currentNode = ++currentNode % this->nodeCount;
			}
			goal = this->node[currentNode];
		}
		else {
			goal.x = goalRange(gen) * range + center.x;
			// goal.y = goalRange(gen) * range;
			goal.z = goalRange(gen) * range + center.z;
		}

		auto pos = transform->GetWorldPosition();
		distance = std::sqrt(std::pow((goal.x - pos.x), 2) + std::pow((goal.y - pos.y), 2));
		arriveTime = distance / speed;
		moveTime = 0;

		direction = Math::Vector3::Subtract(goal, transform->GetWorldPosition());
		direction.y = 0;
		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

		auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };

		auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };
		rotateTimer = 0;
		// 나중에는 이동 불가능한 위치 선택시 false 리턴

		XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		XMFLOAT2 crs;
		XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		if (crs.x >= 0)
			angle.x *= -1;
		return true;
	}
		break;

	case EnemyState::eTRACE:
	{
		if (target == nullptr) {
			DebugNormalMessage("Enemy Controller : target is null");
			return false;
		}
		this->goal = target->GetGameObject()->GetTransform()->GetWorldPosition();
		goal.y = 0;

		direction = Math::Vector3::Subtract(goal, transform->GetWorldPosition());
		direction.y = 0;
		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

		auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };

		auto look = DirectX::XMFLOAT2{ transform->GetWorldLook().x, transform->GetWorldLook().z };
		rotateTimer = 0;

		XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		XMFLOAT2 crs;
		XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		if (crs.x >= 0)
			angle.x *= -1;
		return true;
	}
		break;
	}
	DebugNormalMessage("Enemy Controller : Set Goal() - Invalid Value")
	return false;
}

bool KG::Component::SEnemyControllerComponent::RotateToGoal(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	rotateTimer += elapsedTime;
	float rotateInterval = this->rotateInterval;
	if (state == EnemyState::eTRACE)
		rotateInterval = this->rotateAttackInterval;
	if (rotateInterval <= rotateTimer) {
		return true;
	}
	else {
		DirectX::XMFLOAT4 rot;
		float r = angle.x * elapsedTime / rotateInterval;
		angle.y -= abs(r);
		XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, r, 0));
		gameObject->GetTransform()->Rotate(rot);
		rigid->SetRotation(transform->GetRotation());
	}
	return false;
}

bool KG::Component::SEnemyControllerComponent::MoveToGoal()
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	if (rigid) {
		rigid->SetVelocity(direction, speed);
	}

	if (moveTime / arriveTime > 1)
		return true;

	// 진행 방향
	bool flag_x = false;
	// bool flag_y = false;
	bool flag_z = false;
	auto curPos = gameObject->GetTransform()->GetPosition();
	if (direction.x <= 0) {
		if (curPos.x <= goal.x)
			flag_x = true;
	}
	else {
		if (curPos.x >= goal.x)
			flag_x = true;
	}
	if (direction.z <= 0) {
		if (curPos.z <= goal.z)
			flag_z = true;
	}
	else {
		if (curPos.z >= goal.z)
			flag_z = true;
	}

	if ((flag_x && /*flag_y &&*/ flag_z)) {
		rigid->SetVelocity(direction, 0);
		return true;
	}


	return false;
}

bool KG::Component::SEnemyControllerComponent::Idle(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation) {
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::shotSmallCanon, ANIMSTATE_PLAYING, 0.1f, -1);
		}
	}
	idleTimer += elapsedTime;
	if (idleInterval <= idleTimer)
		return true;
	return false;
}

KG::Component::SEnemyControllerComponent::SEnemyControllerComponent()
{

}

void KG::Component::SEnemyControllerComponent::SetCenter(DirectX::XMFLOAT3 center) {
	this->center = center;
}

void KG::Component::SEnemyControllerComponent::SetSpeed(float speed) {
	this->speed = speed;
}

void KG::Component::SEnemyControllerComponent::SetIdleInterval(float interval) {
	this->idleInterval = interval;
}

void KG::Component::SEnemyControllerComponent::SetRotateInterval(float interval) {
	this->rotateInterval = interval;
}

void KG::Component::SEnemyControllerComponent::SetWanderRange(float range) {
	this->range = range;
}

void KG::Component::SEnemyControllerComponent::SetPosition(DirectX::XMFLOAT3 position)
{
	this->rigid->SetPosition(position);
}

void KG::Component::SEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	// this->SetNetObjectId(this->server->GetNewObjectId());

	this->transform = this->gameObject->GetTransform();
	this->center = this->transform->GetWorldPosition();
	this->rigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();
	if (this->rigid)
		this->rigid->SetRaycastCallback(raycastCallback);
	this->anim = this->gameObject->GetComponent<IAnimationControllerComponent>();
}

void KG::Component::SEnemyControllerComponent::Update(float elapsedTime)
{
	if (hp <= 0) {
		if (!isDead && anim->GetCurrentPlayingAnimationIndex() != KG::Component::MechAnimIndex::dead) {
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::dead, ANIMSTATE_STOP, 0.1, 1);
			auto& cb = this->rigid->GetCollisionBox();
			cb.position.y += 1;
			isDead = true;
		}
		destroyTimer += elapsedTime;
		if (destroyInterval >= destroyTimer) {
			if (rigid) {
				rigid->ReleaseActor();
				rigid = nullptr;
			}
		}
	}
	else {
		switch (this->state) {
		case EnemyState::eWANDER:
			if (SetTarget()) {
				state = EnemyState::eTRACE;
				changedAnimation = false;
				this->action = EnemyAction::eSETGOAL;
				break;
			}
			switch (this->action) {
			case EnemyAction::eIDLE:
				if (Idle(elapsedTime)) {
					changedAnimation = false;
					action = EnemyAction::eSETGOAL;
				}
				break;
			case EnemyAction::eSETGOAL:
				SetGoal();
				action = EnemyAction::eROTATE;
				break;
			case EnemyAction::eROTATE:
				if (RotateToGoal(elapsedTime)) {
					changedAnimation = false;
					action = EnemyAction::eMOVE;
				}
				break;
			case EnemyAction::eMOVE:
				if (MoveToGoal()) {
					changedAnimation = false;
					action = EnemyAction::eIDLE;
					idleTimer = 0;
				}
				else
					moveTime += elapsedTime;
				break;
			default:
				DebugNormalMessage("Enemy State [Wander] : Invalid Enemy Action");
				break;
			}
			break;
		case EnemyState::eTRACE:
			if (!SetTarget()) {
				state = EnemyState::eWANDER;
				action = EnemyAction::eSETGOAL;
				break;
			}
			switch (this->action) {
			case EnemyAction::eSETGOAL:
				SetGoal();
				action = EnemyAction::eROTATE;
				break;
			case EnemyAction::eROTATE:
				if (RotateToGoal(elapsedTime)) {
					changedAnimation = false;
					action = EnemyAction::eATTACK;
				}
				break;
			case EnemyAction::eATTACK:
				if (AttackTarget(elapsedTime)) {
					changedAnimation = false;
					action = EnemyAction::eSETGOAL;
				}
				break;
			}
			break;
		}
	}

	sendTimer += elapsedTime;
	if (sendTimer >= sendInterval) {
		if (this->server) {
			KG::Packet::SC_MOVE_OBJECT p = {};
			p.position = this->transform->GetPosition();
			p.rotation = this->transform->GetRotation();
			this->BroadcastPacket(&p);
		}
		sendTimer = 0;
	}
}

bool KG::Component::SEnemyControllerComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::SEnemyControllerComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		if (this->isUsing()) {
			ImGui::TextDisabled("Center : (%f, %f, %f)", center.x, center.y, center.z);
			ImGui::TextDisabled("Range : %f", range);
			const char* curAction = "None";
			switch (this->action) {
			case EnemyAction::eIDLE:
				curAction = "Idle";
				break;
			case EnemyAction::eSETGOAL:
				curAction = "SetGoal";
				break;
			case EnemyAction::eROTATE:
				curAction = "Rotate";
				break;
			case EnemyAction::eMOVE:
				curAction = "Move";
				break;
			case EnemyAction::eATTACK:
				curAction = "Attack";
				break;
			}
			ImGui::TextDisabled("Action : %s", curAction);
			ImGui::TextDisabled("Goal : (%f, %f, %f)", goal.x, goal.y, goal.z);
			ImGui::TextDisabled("Direction : (%f, %f, %f)", direction.x, direction.y, direction.z);
			ImGui::TextDisabled("Distance : %f", distance);
			ImGui::TextDisabled("MoveTime : %f", moveTime);
			ImGui::TextDisabled("ArriveTime : %f", arriveTime);
			auto angle = transform->GetEulerDegree();
			ImGui::TextDisabled("rotation : (%f, %f, %f)", angle.x, angle.y, angle.z);
			ImGui::TextDisabled("this->angle : (%f, %f)", this->angle.x, this->angle.y);
			ImGui::TextDisabled("Cur Node : (%d)", this->currentNode);
		}
		else {
			std::string cs("position");
			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(cs, this->center);

			std::string rs("range");
			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->range);
		}
		std::string cs("random circuit");
		KG::Utill::ImguiProperty::DrawGUIProperty<bool>(cs, this->randomCircuit);

		ImGui::TextDisabled("Circuit Node [%d]", this->nodeCount);

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);

		XMFLOAT4X4 mat;

		for (int i = 0; i < nodeCount; ++i) {
			// nodeProp[i].OnDrawGUI();
			DirectX::XMStoreFloat4x4(&mat, XMMatrixTranslation(this->node[i].x, this->node[i].y, this->node[i].z));



			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
			std::string bn("Delete Node " + std::to_string(i));
			if (ImGui::Button(bn.c_str()))
			{
				if (i != MAX_NODE - 1) {
					memcpy(&this->node[i], &this->node[i + 1], sizeof(DirectX::XMFLOAT3) * (MAX_NODE - i - 1));
				}
				if (i < currentNode) {
					currentNode--;
				}
				nodeCount--;
				this->action = EnemyAction::eSETGOAL;
			}
		}

		if (nodeCount < MAX_NODE) {
			if (ImGui::Button("Add Node"))
			{
				node[nodeCount++] = { 0,0,0 };
			}
		}

	}
	return false;
}

void KG::Component::SEnemyControllerComponent::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	// for (int i = 0; i < MAX_NODE; ++i)
	// 	nodeProp[i].OnDataLoad(objectElement);
}

void KG::Component::SEnemyControllerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	// auto* componentElement = parentElement->InsertNewChildElement("Component");
	// ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::SEnemyControllerComponent);
	// for (int i = 0; i < MAX_NODE; ++i)
	// 	nodeProp[i].OnDataSave(parentElement);
}

void KG::Component::SEnemyControllerComponent::SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback) {
	this->raycastCallback = callback;
}

void KG::Component::SEnemyControllerComponent::HitBullet() {
	this->hp -= 1;

    KG::Packet::SC_ENEMY_HP hp;
    hp.percentage = float(this->hp) / float(maxHp);
    this->BroadcastPacket(&hp);
}

bool KG::Component::SEnemyControllerComponent::IsDead() const
{
	return isDead;
}

bool KG::Component::SEnemyControllerComponent::IsDelete() const
{
	return destroyInterval <= destroyTimer;
}

bool KG::Component::SEnemyControllerComponent::SetTarget()
{
	if (this->target) {
		if (IsTargetInRange()) {
			return true;
		}
		else
			this->target = nullptr;
	}

	float minVal = FLT_MAX;
	KG::Server::NET_OBJECT_ID minId = -1;
	for (auto& id : playerId) {
		auto* comp = this->server->FindNetObject(id);
		auto pos = comp->GetGameObject()->GetTransform()->GetWorldPosition();
		float d = GetDistance2FromEnemy(pos);
		if (d < minVal) {
			minVal = d;
			minId = id;
		}
	}
	if (IsInTraceRange(minVal)) {
		auto comp = this->server->FindNetObject(minId);
		if (comp == nullptr) {
			playerId.erase(minId);
			target = nullptr;
			return false;
		}
		target = comp;
		return true;
	}
	return false;
}

void KG::Component::SEnemyControllerComponent::RegisterPlayerId(KG::Server::NET_OBJECT_ID id)
{
	if (playerId.count(id) == 0)
		playerId.insert(id);
}

void KG::Component::SEnemyControllerComponent::DeregisterPlayerId(KG::Server::NET_OBJECT_ID id)
{
	if (playerId.count(id) > 0)
		playerId.erase(id);
}

void KG::Component::SEnemyControllerComponent::Destroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	gameObject->Destroy();
}

bool KG::Component::SEnemyControllerComponent::IsTargetInRange() const
{
	if (target) {
		auto pos = target->GetGameObject()->GetTransform()->GetWorldPosition();
		return IsInTraceRange(pos);
	}
	return false;
}

inline void KG::Component::SEnemyControllerComponent::ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime, int repeat) {
	anim->ChangeAnimation(animId, animIndex, nextState, blendingTime, repeat);
	KG::Packet::SC_CHANGE_ANIMATION pa = {};
	pa.animId = animId.value;
	pa.animIndex = animIndex;
	pa.blendingTime = blendingTime;
	pa.nextState = nextState;
	pa.repeat = repeat;
	this->BroadcastPacket(&pa);
	changedAnimation = true;
}

float KG::Component::SEnemyControllerComponent::GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const
{
	auto myPos = this->transform->GetWorldPosition();
	float distance = (pos.x - myPos.x) * (pos.x - myPos.x) + (pos.y - myPos.y) * (pos.y - myPos.y);
	return distance;
}

bool KG::Component::SEnemyControllerComponent::IsInTraceRange(const DirectX::XMFLOAT3 pos) const
{
	float rr = traceRange * traceRange;
	float distance2 = GetDistance2FromEnemy(pos);
	if (rr > distance2)
		return true;
	else
		return false;
}

bool KG::Component::SEnemyControllerComponent::IsInTraceRange(const float distance) const
{
	float rr = range * range;
	return rr > distance;
}

bool KG::Component::SEnemyControllerComponent::AttackTarget(float elapsedTime)
{
	if (attackTimer == 0 && !isInAttackDelay) {
		if (this->anim)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::shotSmallCanon, ANIMSTATE_STOP, 0.1, 1);
		isAttackable = true;
		isInAttackDelay = true;
	}

	if (isInAttackDelay) {
		attackTimer += elapsedTime;
		if (attackTimer >= attackInterval) {
			isInAttackDelay = false;
			attackTimer = 0;
			return true;
		}
	}
	return false;
}

bool KG::Component::SEnemyControllerComponent::IsAttackable() const
{
	return isAttackable;
}

void KG::Component::SEnemyControllerComponent::PostAttack()
{
	isAttackable = false;
}

void KG::Component::SEnemyControllerComponent::Attack(SGameManagerComponent* gameManager)
{
	auto presetName = "Projectile";
	auto presetId = KG::Utill::HashString(presetName);

	auto* scene = this->gameObject->GetScene();
	auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

	auto targetPos = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	targetPos.y += 1;
	auto origin = this->transform->GetPosition();
	origin.y += 3;
	auto direction = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, origin));

	
	KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
	auto tag = KG::Utill::HashString(presetName);
	addObjectPacket.objectTag = tag;
	addObjectPacket.parentTag = 0;
	addObjectPacket.presetId = tag;
	addObjectPacket.position = origin;

	auto id = this->server->GetNewObjectId();
	addObjectPacket.newObjectId = id;
	comp->SetNetObjectId(id);
	this->server->SetServerObject(id, comp);

	// comp->GetGameObject()->GetTransform()->SetScale(0.1, 0.1, 0.1);

	auto projectile = comp->GetGameObject()->GetComponent<SProjectileComponent>();
	
	
	projectile->Initialize(origin, direction, 20, 1);
	this->server->SetServerObject(id, projectile);
	gameManager->BroadcastPacket(&addObjectPacket);
	this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());
	// this->GetGameObject()->GetTransform()->AddChild(comp->GetGameObject()->GetTransform());
	DebugNormalMessage("Enemy Controller : Shot Projectile");
}


bool KG::Component::SEnemyControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

