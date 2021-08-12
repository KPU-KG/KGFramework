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
	goal.y = 0;
	goal.z = mechGoalRange(mechGen) * range + center.z;

	auto v = Math::Vector3::Subtract(goal, this->transform->GetWorldPosition());
	this->goalDistance = v.x * v.x + v.z * v.z;
	this->moveDistance = 0;

	return true;
}

bool KG::Component::SEnemyMechComponent::Rotate(float elapsedTime)
{

	if (this->target == nullptr)
		return true;

	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING, 0.1f, -1);
	}


	if (!this->target->isUsing()) {
		this->target = nullptr;
		return true;
	}

	auto look = this->transform->GetWorldLook();
	auto dir = Math::Vector3::Subtract(this->goal, this->transform->GetWorldPosition());

	look.y = 0;
	dir.y = 0;

	XMFLOAT3 angle;
	XMStoreFloat3(&angle, XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	auto speed = this->wanderRotateSpeed;
	
	if (stateManager->GetCurState() == MechStateManager::STATE_TRACE) {
		speed = this->traceRotateSpeed;
	}

	float amount = min(abs(angle.x), speed * elapsedTime);

	if (angle.x < 0)
		amount *= -1;

	if (crs.y >= 0) {
		amount *= -1;
	}

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), amount));
	gameObject->GetTransform()->Rotate(rot);
	rigid->SetRotation(transform->GetRotation());

	if (amount == abs(angle.x)) {
		return true;
	}

	return false;
}

bool KG::Component::SEnemyMechComponent::Move(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	if (noObstacleInAttack)
		return true;

	auto spd = this->wanderMoveSpeed;
	if (this->stateManager->GetCurState() == MechStateManager::STATE_TRACE)
		spd += this->traceMoveSpeed;


	auto dir = Math::Vector3::Normalize(Math::Vector3::Subtract(goal, this->transform->GetWorldPosition()));

	if (rigid) {
		rigid->SetVelocity(dir, spd);
	}

	this->moveDistance += elapsedTime * spd;

	if (this->moveDistance >= this->goalDistance) {
		this->moveDistance = 0;
		rigid->SetVelocity(XMFLOAT3{ 0,0,0 }, 0);
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
	hp = maxHp;
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
			ImGui::TextDisabled("Distance : %f", goalDistance);
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
	if (this->target != nullptr) {
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
		return true;
	}

	if (!this->target->isUsing()) {
		this->target = nullptr;
		return true;
	}

	if (noObstacleInAttack) {
		this->goal = target->GetGameObject()->GetTransform()->GetWorldPosition();
		this->goal.y = 0;
		this->isAttackRotation = true;
	}

	return true;
}

bool KG::Component::SEnemyMechComponent::IsTargetInRange() const
{
	if (target) {
		if (target->isUsing()) {
			auto pos = target->GetGameObject()->GetTransform()->GetWorldPosition();
			return IsInTraceRange(pos);
		}
		else {
			return false;
		}
	}
	return false;
}

bool KG::Component::SEnemyMechComponent::CheckAttackable()
{
	noObstacleInAttack = false;

	if (this->target == nullptr) {
		return true;
	}

	if (!this->target->isUsing()) {
		this->target = nullptr;
		return true;
	}


	auto pos = this->transform->GetWorldPosition();
	pos.y = 0.5;

	XMFLOAT3 dir = Math::Vector3::Normalize(Math::Vector3::Subtract(this->target->GetGameObject()->GetTransform()->GetWorldPosition(), pos));
	uint32_t mask = static_cast<uint32_t>(FilterGroup::ePLAYER);
	mask |= static_cast<uint32_t>(FilterGroup::eBUILDING);
	mask |= static_cast<uint32_t>(FilterGroup::eBOX);
	
	DebugNormalMessage("dir : " << dir);
	DebugNormalMessage("mask : " << mask);
	DebugNormalMessage("pos : " << pos);

	auto comp = this->rigid->GetScene()->QueryRaycast(pos, dir, attackRange, this->rigid->GetId(), mask);
	if (comp != nullptr) {
		auto filter = comp->GetFilterGroup();
		if (filter & static_cast<uint32_t>(FilterGroup::ePLAYER)) {
			noObstacleInAttack = true;
		}
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

	bool operator<(const Coord& r) const{
		if ((g + h) == (r.g + r.h)) {
			if (x == r.x)
				return z > r.z;
			else
				return x > r.z;
		}
		else
			return (g + h) > (r.g + r.h);
	}

	bool operator==(const Coord& r) const {
		return (x == r.x && z == r.z);
	}
};

bool KG::Component::SEnemyMechComponent::CheckRoot()
{
	isMovableInTrace = false;
	float minDist = FLT_MAX;
	int mx = INT16_MAX;
	int mz = INT16_MAX;

	auto myPos = this->transform->GetWorldPosition();

	if (this->target == nullptr) {
		return true;
	}
	if (this->target->isUsing()) {
		this->target = nullptr;
		return true;
	}

	auto targetPos = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	for (int tx = -10; tx < 10; ++tx) {
		for (int tz = -10; tz < 10; ++tz) {

			// auto center = this->transform->GetWorldPosition();
			int dx = myPos.x + tx;
			int dz = myPos.z + tz;

			// 해당 위치가 맵 밖이면 체크 x
			if (dx + (MAP_SIZE_X / 2) < 0 || dz + (MAP_SIZE_Z / 2) < 0 || dx + (MAP_SIZE_X / 2) >= MAP_SIZE_X || dz + (MAP_SIZE_Z / 2) >= MAP_SIZE_Z)
				continue;

			// 해당 위치가 건물이 있으면 체크 x
			if (session[dx + (MAP_SIZE_X / 2)][dz + (MAP_SIZE_Z / 2)])
				continue;

			XMFLOAT3 pathPos;

			pathPos.x = dx;
			pathPos.y = targetPos.y;
			pathPos.z = dz;

			myPos.y = pathPos.y;

			uint32_t mask = static_cast<uint32_t>(FilterGroup::ePLAYER);
			mask |= static_cast<uint32_t>(FilterGroup::eBUILDING);
			mask |= static_cast<uint32_t>(FilterGroup::eBOX);

			auto dir = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, pathPos));
			auto comp = this->rigid->GetScene()->QueryRaycast(pathPos, dir, attackRange, this->rigid->GetId(), mask);
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

	// cal root
	// 그리디 알고리즘으로 갈 수 있는지 체크 (가능하면 감)
	// A* 사용

	XMFLOAT3 pathPos{ (float)mx, targetPos.y, (float)mz };
	auto v = Math::Vector3::Subtract(myPos, pathPos);
	XMFLOAT3 dir = Math::Vector3::Normalize(v);
	auto dist = sqrt(v.x * v.x + v.z * v.z);
	auto comp = this->rigid->GetScene()->QueryRaycast(pathPos, dir, dist, this->rigid->GetId());
	if (comp == nullptr) {
		this->goal.x = mx;
		this->goal.z = mz;

		// set rotation var
		auto v = Math::Vector3::Subtract(goal, this->transform->GetWorldPosition());
		this->goalDistance = v.x * v.x + v.z * v.z;

		isMovableInTrace = true;
		return true;
	}/*
	else {
		std::set<Coord> open_list;
		std::vector<Coord> closed_list;
		Coord g(targetPos.x + (MAP_SIZE_X / 2), targetPos.z + (MAP_SIZE_Z / 2));
		Coord s(myPos.x + (MAP_SIZE_X / 2), myPos.z + (MAP_SIZE_Z / 2));
		open_list.insert(s);
		int count = 0;
		while (!open_list.empty()) {
			if (count++ > 3000)
				return false;
			auto c = *(open_list.begin());
			if (c == g) {
				Coord* cur = c.parent;
				while (cur->parent != nullptr) {
					path.emplace_back(cur->x, cur->z);
					cur = c.parent;
				}

				isPathFinding = true;
				return true;
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

						Coord newCoord(static_cast<int>(x + (MAP_SIZE_X / 2)), static_cast<int>(z + (MAP_SIZE_Z / 2)));

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

						bool flag = false;
						for (auto ol : open_list) {
							if (ol == newCoord) {
								flag = true;
								if (newCoord.g < ol.g) {
									ol.g = newCoord.g;
									ol.parent = newCoord.parent;
								}
							}
						}
						if (!flag) {
							open_list.insert(newCoord);
						}
					}
				}
				closed_list.emplace_back(c);
				open_list.erase(c);
			}
		}*/
	return true;
	// }
}

bool KG::Component::SEnemyMechComponent::IsMobableInTrace() const
{
	return isMovableInTrace;
}

bool KG::Component::SEnemyMechComponent::IsAttackRotation() const
{
	return isAttackRotation;
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
	auto v = Math::Vector3::Subtract(this->transform->GetWorldPosition(), pos);
	return v.x * v.x + v.z * v.z;
}

bool KG::Component::SEnemyMechComponent::IsInTraceRange(const DirectX::XMFLOAT3 pos) const
{
	float rr = traceRange * traceRange;
	float distance2 = GetDistance2FromEnemy(pos);
	return rr > distance2;
}

bool KG::Component::SEnemyMechComponent::IsInTraceRange(const float distance2) const
{
	float rr = traceRange * traceRange;
	return rr > distance2;
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
			isAttackRotation = false;
			attackTimer = 0;
			return true;
		}
	}
	return false;
}

void KG::Component::SEnemyMechComponent::Attack(SGameManagerComponent* gameManager)
{
	// auto presetName = "Projectile";
	float interval = 2.6;
	for (int i = 0; i < 2; ++i) {
		if (this->target == nullptr) {
			return;
		}

		if (!this->target->isUsing()) {
			this->target = nullptr;
			return;
		}
		auto presetName = "Missile";
		auto presetId = KG::Utill::HashString(presetName);

		auto* scene = this->gameObject->GetScene();
		auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

		
		auto targetPos = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
		targetPos.y += 1;
		auto origin = this->transform->GetWorldPosition();
		origin.y += 4;
		auto right = this->transform->GetWorldRight();
		right.y = 0;

		auto direction = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, origin));

		XMStoreFloat3(&right, Math::Vector3::XMVectorScale(XMLoadFloat3(&right), (-interval / 2 + interval * i)));
		origin = Math::Vector3::Add(origin, right);


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


		projectile->Initialize(origin, direction, 25, 1);
		projectile->SetTargetPosition(targetPos);

		this->server->SetServerObject(id, projectile);
		gameManager->BroadcastPacket(&addObjectPacket);
		this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());

		DebugNormalMessage("Enemy Mech : Shot Projectile");
	}
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
	bool result = dynamic_cast<SEnemyMechComponent*>(enemyComp)->Move(elapsedTime);
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->SetIdleTime(0);
	return result;
}

void KG::Component::MechMoveAction::EndAction() {
	// if (!dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsPathFinding()) {
	dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
	// }
}

bool KG::Component::MechRotateAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyMechComponent*>(enemyComp)->Rotate(elapsedTime);
}

void KG::Component::MechRotateAction::EndAction() {
	// if (!dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsPathFinding()) {
		dynamic_cast<SEnemyMechComponent*>(enemyComp)->ReadyNextAnimation(false);
	// }
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
			if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsMobableInTrace()) {
				curAction = TRACE_ACTION_ROTATE;
			}
			else {
				curAction = TRACE_ACTION_SET_TARGET_ROTATION;
			}
			break;

		case TRACE_ACTION_ROTATE:
			if (dynamic_cast<SEnemyMechComponent*>(enemyComp)->IsAttackRotation()) {
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
