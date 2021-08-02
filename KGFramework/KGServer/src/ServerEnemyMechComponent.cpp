#include "pch.h"
#include "ServerEnemyMechComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IPhysicsScene.h"
#include "IAnimationComponent.h"
#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include <queue>
#include "Scene.h"


std::random_device mechRd;
std::mt19937 mechGen(mechRd());
std::uniform_real_distribution<float> mechGoalRange(-0.5, 0.5);

bool KG::Component::SEnemyMechComponent::SetGoal()
{
	goal.x = mechGoalRange(mechGen) * range + center.x;
	goal.z = mechGoalRange(mechGen) * range + center.z;

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

bool KG::Component::SEnemyMechComponent::RotateToGoal(float elapsedTime)
{
	if (target) {
		std::pair<int, int> curTarget(round(target->GetGameObject()->GetTransform()->GetWorldPosition().x), round(target->GetGameObject()->GetTransform()->GetWorldPosition().z));
		if (curTarget == prevTarget)
			return true;
	}

	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	rotateTimer += elapsedTime;
	float rotateInterval = this->rotateInterval;

	if (stateManager->GetCurState() == MechStateManager::STATE_TRACE) {
		rotateInterval = this->rotateAttackInterval;
	}

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

bool KG::Component::SEnemyMechComponent::MoveToGoal(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	if (rigid) {
		rigid->SetVelocity(direction, speed);
	}

	//////////////////////// fix
	moveTime += elapsedTime;

	if (moveTime / arriveTime >= 1)
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

bool KG::Component::SEnemyMechComponent::Idle(float elapsedTime)
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

KG::Component::SEnemyMechComponent::SEnemyMechComponent()
{

}

void KG::Component::SEnemyMechComponent::OnCreate(KG::Core::GameObject* obj)
{
	SEnemyUnitComponent::OnCreate(obj);
	this->stateManager = new MechStateManager(this);
	stateManager->Init();
}

void KG::Component::SEnemyMechComponent::Update(float elapsedTime)
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
		stateManager->Execute(elapsedTime);
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

void KG::Component::SEnemyMechComponent::HitBullet() {
	this->hp -= 1;

	KG::Packet::SC_ENEMY_HP hp;
	hp.percentage = float(this->hp) / float(maxHp);
	this->BroadcastPacket(&hp);
}

bool KG::Component::SEnemyMechComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::SEnemyMechComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		if (this->isUsing()) {
			ImGui::TextDisabled("Center : (%f, %f, %f)", center.x, center.y, center.z);
			ImGui::TextDisabled("Range : %f", range);
			ImGui::TextDisabled("Goal : (%f, %f, %f)", goal.x, goal.y, goal.z);
			ImGui::TextDisabled("Direction : (%f, %f, %f)", direction.x, direction.y, direction.z);
			ImGui::TextDisabled("Distance : %f", distance);
			ImGui::TextDisabled("MoveTime : %f", moveTime);
			ImGui::TextDisabled("ArriveTime : %f", arriveTime);
			auto angle = transform->GetEulerDegree();
			ImGui::TextDisabled("rotation : (%f, %f, %f)", angle.x, angle.y, angle.z);
			ImGui::TextDisabled("this->angle : (%f, %f)", this->angle.x, this->angle.y);
		}
		else {
			std::string cs("position");
			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(cs, this->center);

			std::string rs("range");
			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->range);
		}

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
	}
	return false;
}



bool KG::Component::SEnemyMechComponent::SetTarget()
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

void KG::Component::SEnemyMechComponent::Destroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	gameObject->Destroy();
}

bool KG::Component::SEnemyMechComponent::SetAttackRotation()
{
	if (target == nullptr) {
		DebugNormalMessage("Enemy Controller : target is null");
		return false;
	}

	if (isPathFinding) {
		if (path.size() > 0) {
			this->goal.x = path[0].first;
			this->goal.z = path[0].second;
			path.erase(path.begin());
		}
	}
	else {
		this->goal = target->GetGameObject()->GetTransform()->GetWorldPosition();
		goal.y = 0;
	}

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

bool KG::Component::SEnemyMechComponent::IsTargetInRange() const
{
	if (target) {
		auto pos = target->GetGameObject()->GetTransform()->GetWorldPosition();
		return IsInTraceRange(pos);
	}
	return false;
}

bool KG::Component::SEnemyMechComponent::CheckAttackable()
{
	noObstacleInAttack = false;
	XMFLOAT3 dir = Math::Vector3::Normalize(Math::Vector3::Subtract(this->target->GetGameObject()->GetTransform()->GetWorldPosition(), this->transform->GetWorldPosition()));
	uint32_t mask = static_cast<uint32_t>(FilterGroup::ePLAYER);
	mask |= static_cast<uint32_t>(FilterGroup::eBUILDING);
	mask |= static_cast<uint32_t>(FilterGroup::eBOX);
	
	auto comp = this->rigid->GetScene()->QueryRaycast(this->transform->GetWorldPosition(), dir, attackRange, this->rigid->GetId(), mask);
	if (comp != nullptr) {
		// raycast 쓰는거라 탄 날아가는 중에 검사하면 플레이어가 안나오네 ㅋㅋ
		auto filter = comp->GetFilterGroup();
		if (filter & static_cast<uint32_t>(FilterGroup::ePLAYER)) {
			noObstacleInAttack = true;
		}
	}
	if (noObstacleInAttack) {
		isPathFinding = false;
		path.clear();
	}
	return true;
}

bool KG::Component::SEnemyMechComponent::NoObstacleInAttack() const
{
	return noObstacleInAttack;
}

struct Coord {
	int x, z;
	int g = 0, h = 0;
	Coord* parent = nullptr;
	Coord(int x, int z) : x(x), z(z) {}
	Coord(float x, float z) : x(round(x)), z(round(z)) {}

	bool operator<(const Coord& r) {
		return (g + h) > (r.g + r.h);
	}

	bool operator==(const Coord& r) {
		return (x == r.x && z == r.z);
	}
};

bool KG::Component::SEnemyMechComponent::CheckRoot()
{
	isMovableInTrace = false;
	// 일단 십자로만?
	float minDist = FLT_MAX;
	int mx = INT16_MAX;
	int mz = INT16_MAX;

	auto myPos = this->transform->GetWorldPosition();
	auto targetPos = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	for (int tx = -10; tx < 10; ++tx) {
		for (int tz = -10; tz < 10; ++tz) {

			// auto center = this->transform->GetWorldPosition();

			int dx = myPos.x + tx;
			int dz = myPos.z + tz;

			// 해당 위치가 맵 밖이면 체크 x
			if (dx < 0 || dz < 0 || dx >= MAP_SIZE_X || dz >= MAP_SIZE_Z)
				continue;

			// 해당 위치가 건물이 있으면 체크 x
			if (session[dx][dz])
				continue;

			XMFLOAT3 pathPos;

			pathPos.x = dx;
			pathPos.y = targetPos.y;
			pathPos.z = dz;

			myPos.y = pathPos.y;

			auto dir = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, pathPos));
			auto comp = this->rigid->GetScene()->QueryRaycast(pathPos, dir, attackRange, this->rigid->GetId());
			if (comp == nullptr)
				continue;

			auto pathVec = Math::Vector3::Subtract(myPos, pathPos);
			auto pathDir = Math::Vector3::Normalize(pathVec);
			float pathDist = sqrt(pathVec.x * pathVec.x + pathVec.z * pathVec.z);
			auto pathComp = this->rigid->GetScene()->QueryRaycast(myPos, pathDir, pathDist, this->rigid->GetId());
			if (pathComp != nullptr)
				continue;
			// 여기서 센터까지 레이캐스트로 아무것도 없으면 그냥 거리로 계산
			// 아니면 a*로 플레이어 위치 잡고 가면서 노드마다 한 번 더 체크

			auto filter = comp->GetFilterGroup();
			if (filter & static_cast<uint32_t>(FilterGroup::ePLAYER)) {
				float dist = tx * tx + tz * tz;
				if (minDist > dist) {
					minDist = dist;
					mx = dx;
					mz = dz;
				}
			}
		}
	}

	// if (minDist == FLT_MAX) {
	// 	return false;
	// }

	// cal root
	// 그리디 알고리즘으로 갈 수 있는지 체크 (가능하면 감)
	// A* 사용

	XMFLOAT3 pathPos{ (float)mx, targetPos.y, (float)mz };
	auto v = Math::Vector3::Subtract(myPos, pathPos);
	XMFLOAT3 dir = Math::Vector3::Normalize(v);
	auto dist = sqrt(v.x * v.x + v.z * v.z);
	auto comp = this->rigid->GetScene()->QueryRaycast(pathPos, dir, dist, this->rigid->GetId());
	if (comp == nullptr) {
		goal.x = mx;
		goal.z = mz;

		// set rotation var
		distance = std::sqrt(std::pow((goal.x - myPos.x), 2) + std::pow((goal.z - myPos.z), 2));
		arriveTime = distance / (speed + 3);
		moveTime = 0;
		
		direction = Math::Vector3::Subtract(goal, transform->GetWorldPosition());
		direction.y = 0;
		XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));
		
		auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };
		
		auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };
		rotateTimer = 0;
		
		XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		XMFLOAT2 crs;
		XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
		if (crs.x >= 0)
			angle.x *= -1;
		isMovableInTrace = true;
		return true;
	}
	else {
		std::set<Coord> open_list;
		std::vector<Coord> closed_list;
		Coord g(targetPos.x, targetPos.z);
		while (!open_list.empty()) {
			// timer 재는 것도 나와야 함
			auto c = *(open_list.begin());
			if (c == g) {
				Coord* cur = c.parent;
				while (cur->parent != nullptr) {
					path.emplace_back(cur->x, cur->z);
					cur = c.parent;
				}

				isPathFinding = true;
				isMovableInTrace = true;
				return true;
				// push path
				// pathfinding = true
				// break;
			}
			else {
				for (int dx = -1; dx < 2; ++dx) {
					int x = c.x + dx;

					if (x < 0 || x >= MAP_SIZE_X) 
						continue;
					
					for (int dz = -1; dz < 2;++dz) {
						int z = c.z + dz;

						if (z < 0 || z >= MAP_SIZE_Z) 
							continue;

						if (session[x][z])
							continue;
						
						Coord newCoord(x, z);

						bool is_closed = false;
						for (auto cl : closed_list) {
							if (cl == newCoord) {
								is_closed = true;
								break;
							}
						}
						if (is_closed)
							continue;

						newCoord.h = g.x * 10 + g.z * 10;
						if (dx != 0 && dz != 0)
							newCoord.g = c.g + 14;
						else
							newCoord.g = c.g + 10;
						newCoord.parent = &c;

						for (auto ol : open_list) {
							if (ol == newCoord) {
								if (newCoord.g < ol.g) {
									ol.g = newCoord.g;
									ol.parent = newCoord.parent;
								}
							}
						}
					}
				}
			}
		}
		return true;
	}
}

bool KG::Component::SEnemyMechComponent::IsMobableInTrace() const
{
	return isMovableInTrace;
}

bool KG::Component::SEnemyMechComponent::IsPathFinding() const
{
	return isPathFinding;
}


inline void KG::Component::SEnemyMechComponent::ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime, int repeat) {
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

float KG::Component::SEnemyMechComponent::GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const
{
	auto myPos = this->transform->GetWorldPosition();
	float distance = (pos.x - myPos.x) * (pos.x - myPos.x) + (pos.y - myPos.y) * (pos.y - myPos.y);
	return distance;
}

bool KG::Component::SEnemyMechComponent::IsInTraceRange(const DirectX::XMFLOAT3 pos) const
{
	float rr = traceRange * traceRange;
	float distance2 = GetDistance2FromEnemy(pos);
	return rr > distance2;
}

bool KG::Component::SEnemyMechComponent::IsInTraceRange(const float distance) const
{
	float rr = traceRange * traceRange;
	return rr > distance;
}

bool KG::Component::SEnemyMechComponent::AttackTarget(float elapsedTime)
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

void KG::Component::SEnemyMechComponent::Attack(SGameManagerComponent* gameManager)
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

	auto projectile = comp->GetGameObject()->GetComponent<SProjectileComponent>();


	projectile->Initialize(origin, direction, 20, 1);
	this->server->SetServerObject(id, projectile);
	gameManager->BroadcastPacket(&addObjectPacket);
	this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());
	DebugNormalMessage("Enemy Mech : Shot Projectile");

	prevTarget.first = round(this->target->GetGameObject()->GetTransform()->GetWorldPosition().x);
	prevTarget.second = round(this->target->GetGameObject()->GetTransform()->GetWorldPosition().z);
}

bool KG::Component::MechIdleAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->Idle(elapsedTime);
}

void KG::Component::MechIdleAction::EndAction() {
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::MechSetGoalAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->SetGoal();
}

void KG::Component::MechSetGoalAction::EndAction() {

}

bool KG::Component::MechSetTargetAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->SetAttackRotation();
}

void KG::Component::MechSetTargetAction::EndAction() {

}

bool KG::Component::MechMoveAction::Execute(float elapsedTime) {
	bool result = dynamic_cast<SEnemyMechComponent*>(enemyComp)->MoveToGoal(elapsedTime);
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->SetIdleTime(0);
	return result;
}

void KG::Component::MechMoveAction::EndAction() {
	if (!dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsPathFinding()) {
		dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
	}
}

bool KG::Component::MechRotateAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->RotateToGoal(elapsedTime);
}

void KG::Component::MechRotateAction::EndAction() {
	if (!dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsPathFinding()) {
		dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
	}
}

bool KG::Component::MechAttackAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->AttackTarget(elapsedTime);
}

void KG::Component::MechAttackAction::EndAction() {
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::MechCheckAttackableAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->CheckAttackable();
}

void KG::Component::MechCheckAttackableAction::EndAction()
{

}

bool KG::Component::MechCheckRootAction::Execute(float elapsedTime)
{
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->CheckRoot();
	return true;
}

void KG::Component::MechCheckRootAction::EndAction()
{
}

KG::Component::MechWanderState::~MechWanderState() {
	for (auto& a : action) {
		delete a;
	}
}

void KG::Component::MechWanderState::InitState() {
	action[WANDER_ACTION_IDLE] = new MechIdleAction(enemyComp);
	action[WANDER_ACTION_SET_GOAL] = new MechSetGoalAction(enemyComp);
	action[WANDER_ACTION_ROTATE] = new MechRotateAction(enemyComp);
	action[WANDER_ACTION_MOVE] = new MechMoveAction(enemyComp);
}

void KG::Component::MechWanderState::Execute(float elapsedTime) {
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case WANDER_ACTION_IDLE:
			curAction = WANDER_ACTION_SET_GOAL;
			break;
		case WANDER_ACTION_SET_GOAL:
			curAction = WANDER_ACTION_ROTATE;
			break;
		case WANDER_ACTION_ROTATE:
			curAction = WANDER_ACTION_MOVE;
			break;
		case WANDER_ACTION_MOVE:
			curAction = WANDER_ACTION_IDLE;
			break;
		}
	}
}

float KG::Component::MechWanderState::GetValue() {
	return 1;
}

KG::Component::MechTraceState::~MechTraceState()
{
	for (auto& act : action)
		delete act;
}

void KG::Component::MechTraceState::InitState() {
	action[TRACE_ACTION_SET_TARGET_ROTATION] = new MechSetTargetAction(enemyComp);
	action[TRACE_ACTION_ROTATE] = new MechRotateAction(enemyComp);
	action[TRACE_ACTION_ATTACK] = new MechAttackAction(enemyComp);
	action[TRACE_ACTION_CHECK_ATTACKABLE] = new MechCheckAttackableAction(enemyComp);
	action[TRACE_ACTION_CHECK_ROOT] = new MechCheckRootAction(enemyComp);
	action[TRACE_ACTION_MOVE] = new MechMoveAction(enemyComp);
}

// if (attackable) -> set rotation -> rotate -> attack
//            else -> search root  -> if (movable) -> rotate -> move -> attack
//                                            else -> return to spawn position

void KG::Component::MechTraceState::Execute(float elapsedTime) {
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case TRACE_ACTION_CHECK_ATTACKABLE:
			if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->NoObstacleInAttack()) {
				curAction = TRACE_ACTION_SET_TARGET_ROTATION;
			}
			else {
				curAction = TRACE_ACTION_CHECK_ROOT;
			}
			break;
		case TRACE_ACTION_SET_TARGET_ROTATION:
			curAction = TRACE_ACTION_ROTATE;
			break;

		case TRACE_ACTION_CHECK_ROOT:
			if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsPathFinding()) {
				curAction = TRACE_ACTION_SET_TARGET_ROTATION;
			}
			else {
				curAction = TRACE_ACTION_ROTATE;
			}
			break;
		// case check root
		// if movable
			// cur action = rotate
		// else
			// return to spawn position

		case TRACE_ACTION_ROTATE:
			if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->NoObstacleInAttack()) {
				curAction = TRACE_ACTION_ATTACK;
			}
			else {
				curAction = TRACE_ACTION_MOVE;
			}
			break;
		case TRACE_ACTION_ATTACK:
			curAction = TRACE_ACTION_CHECK_ATTACKABLE;
			break;

		case TRACE_ACTION_MOVE:
			curAction = TRACE_ACTION_CHECK_ATTACKABLE;
			break;
		}
	}
}

float KG::Component::MechTraceState::GetValue() {
	if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->SetTarget())
		return 2;
	return 0;
}

KG::Component::MechStateManager::MechStateManager(SEnemyMechComponent* comp) : enemyComp(comp) 
{

}

KG::Component::MechStateManager::~MechStateManager() {
	for (auto& s : state) {
		delete s;
	}
}

void KG::Component::MechStateManager::Init() {
	state[STATE_WANDER] = new MechWanderState(enemyComp);
	state[STATE_TRACE] = new MechTraceState(enemyComp);
	for (auto& s : state) {
		s->InitState();
	}
	curState = STATE_WANDER;
}

void KG::Component::MechStateManager::SetState() {
	int max = INT_MIN;
	int idx = -1;
	for (int i = 0; i < STATE_COUNT; ++i) {
		int v = state[i]->GetValue();
		if (v > max) {
			max = v;
			idx = i;
		}
	}

	if (curState != idx) {
		curState = idx;
	}
}

void KG::Component::MechStateManager::Execute(float elapsedTime) {
	SetState();
	state[curState]->Execute(elapsedTime);
}

int KG::Component::MechStateManager::GetCurState() const
{
	return curState;
}
