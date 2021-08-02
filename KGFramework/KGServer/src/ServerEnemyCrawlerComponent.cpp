
#include "pch.h"

#include "ServerEnemyCrawlerComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IAnimationComponent.h"
#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include "Scene.h"


std::random_device crawlerRd;
std::mt19937 crawlerGen(crawlerRd());
std::uniform_real_distribution<float> crawlerGoalRange(-0.5, 0.5);

bool KG::Component::SEnemyCrawlerComponent::SetGoal()
{

	goal.x = crawlerGoalRange(crawlerGen) * range + center.x;
	goal.z = crawlerGoalRange(crawlerGen) * range + center.z;

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

bool KG::Component::SEnemyCrawlerComponent::RotateToGoal(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	rotateTimer += elapsedTime;
	float rotateInterval = this->rotateInterval;

	if (stateManager->curState == CrawlerStateManager::STATE_TRACE)
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

bool KG::Component::SEnemyCrawlerComponent::MoveToGoal(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation)
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	if (rigid) {
		rigid->SetVelocity(direction, speed);
	}

	//////////////////////// fix
	moveTime += elapsedTime;

	if (moveTime / arriveTime > 1)
		return true;

	// 진행 방향
	bool flag_x = false;
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

	if ((flag_x && flag_z)) {
		rigid->SetVelocity(direction, 0);
		return true;
	}


	return false;
}

bool KG::Component::SEnemyCrawlerComponent::Idle(float elapsedTime)
{
	if (anim) {
		if (!changedAnimation) {
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::idle, ANIMSTATE_PLAYING, 0.1f, -1);
		}
	}
	idleTimer += elapsedTime;
	if (idleInterval <= idleTimer)
		return true;
	return false;
}

KG::Component::SEnemyCrawlerComponent::SEnemyCrawlerComponent()
{

}

void KG::Component::SEnemyCrawlerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SEnemyUnitComponent::OnCreate(obj);
	this->stateManager = new CrawlerStateManager(this);
	stateManager->Init();
}

void KG::Component::SEnemyCrawlerComponent::Update(float elapsedTime)
{
	if (hp <= 0) {
		if (!isDead && anim->GetCurrentPlayingAnimationIndex() != KG::Component::CrawlerAnimIndex::idle) {
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::idle, ANIMSTATE_STOP, 0.1, 1);
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

void KG::Component::SEnemyCrawlerComponent::HitBullet() {
	this->hp -= 1;

	KG::Packet::SC_ENEMY_HP hp;
	hp.percentage = float(this->hp) / float(maxHp);
	this->BroadcastPacket(&hp);
}

bool KG::Component::SEnemyCrawlerComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::SEnemyCrawlerComponent>()) {
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



bool KG::Component::SEnemyCrawlerComponent::SetTarget()
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

void KG::Component::SEnemyCrawlerComponent::Destroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	gameObject->Destroy();
}

bool KG::Component::SEnemyCrawlerComponent::SetAttackRotation()
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

	// return false;
}

bool KG::Component::SEnemyCrawlerComponent::IsTargetInRange() const
{
	if (target) {
		auto pos = target->GetGameObject()->GetTransform()->GetWorldPosition();
		return IsInTraceRange(pos);
	}
	return false;
}

inline void KG::Component::SEnemyCrawlerComponent::ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime, int repeat) {
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

float KG::Component::SEnemyCrawlerComponent::GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const
{
	auto myPos = this->transform->GetWorldPosition();
	float distance = (pos.x - myPos.x) * (pos.x - myPos.x) + (pos.y - myPos.y) * (pos.y - myPos.y);
	return distance;
}

bool KG::Component::SEnemyCrawlerComponent::IsInTraceRange(const DirectX::XMFLOAT3 pos) const
{
	float rr = traceRange * traceRange;
	float distance2 = GetDistance2FromEnemy(pos);
	return rr > distance2;
}

bool KG::Component::SEnemyCrawlerComponent::IsInTraceRange(const float distance) const
{
	float rr = traceRange * traceRange;
	return rr > distance;
}

bool KG::Component::SEnemyCrawlerComponent::AttackTarget(float elapsedTime)
{
	if (attackTimer == 0 && !isInAttackDelay) {
		if (this->anim)
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::idle, ANIMSTATE_STOP, 0.1, 1);
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

void KG::Component::SEnemyCrawlerComponent::Attack(SGameManagerComponent* gameManager)
{
	// 나중에 수정 해야 함

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
	DebugNormalMessage("Enemy Controller : Shot Projectile");
}

bool KG::Component::CrawlerIdleAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->Idle(elapsedTime);
}

void KG::Component::CrawlerIdleAction::EndAction() {
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::CrawlerSetGoalAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetGoal();
}

void KG::Component::CrawlerSetGoalAction::EndAction() {

}

bool KG::Component::CrawlerSetTargetAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetAttackRotation();
}

void KG::Component::CrawlerSetTargetAction::EndAction() {

}

bool KG::Component::CrawlerMoveAction::Execute(float elapsedTime) {
	bool result = dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->MoveToGoal(elapsedTime);
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetIdleTime(0);
	return result;
}

void KG::Component::CrawlerMoveAction::EndAction() {

}

bool KG::Component::CrawlerRotateAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->RotateToGoal(elapsedTime);
}

void KG::Component::CrawlerRotateAction::EndAction() {
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::CrawlerAttackAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->AttackTarget(elapsedTime);
}

void KG::Component::CrawlerAttackAction::EndAction() {
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

KG::Component::CrawlerWanderState::~CrawlerWanderState() {
	for (auto& a : action) {
		delete a;
	}
}

void KG::Component::CrawlerWanderState::InitState() {
	action[WANDER_ACTION_IDLE] = new CrawlerIdleAction(enemyComp);
	action[WANDER_ACTION_SETGOAL] = new CrawlerSetGoalAction(enemyComp);
	action[WANDER_ACTION_ROTATE] = new CrawlerRotateAction(enemyComp);
	action[WANDER_ACTION_MOVE] = new CrawlerMoveAction(enemyComp);
}

void KG::Component::CrawlerWanderState::Execute(float elapsedTime) {
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case WANDER_ACTION_IDLE:
			curAction = WANDER_ACTION_SETGOAL;
			break;
		case WANDER_ACTION_SETGOAL:
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

float KG::Component::CrawlerWanderState::GetValue() {
	return 1;
}

KG::Component::CrawlerTraceState::~CrawlerTraceState()
{
	for (auto& act : action)
		delete act;
}

void KG::Component::CrawlerTraceState::InitState() {
	action[TRACE_ACTION_SETGOAL] = new CrawlerSetTargetAction(enemyComp);
	action[TRACE_ACTION_ROTATE] = new CrawlerRotateAction(enemyComp);
	action[TRACE_ACTION_ATTACK] = new CrawlerAttackAction(enemyComp);
}

void KG::Component::CrawlerTraceState::Execute(float elapsedTime) {
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case TRACE_ACTION_SETGOAL:
			curAction = TRACE_ACTION_ROTATE;
			break;
		case TRACE_ACTION_ROTATE:
			curAction = TRACE_ACTION_ATTACK;
			break;
		case TRACE_ACTION_ATTACK:
			curAction = TRACE_ACTION_SETGOAL;
			break;
		}
	}
}

float KG::Component::CrawlerTraceState::GetValue() {
	if (dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetTarget())
		return 2;
	return 0;
}

KG::Component::CrawlerStateManager::CrawlerStateManager(SEnemyUnitComponent* comp) : enemyComp(comp) { }

KG::Component::CrawlerStateManager::~CrawlerStateManager() {
	for (auto& s : state) {
		delete s;
	}
}

void KG::Component::CrawlerStateManager::Init() {
	state[STATE_WANDER] = new CrawlerWanderState(enemyComp);
	state[STATE_TRACE] = new CrawlerTraceState(enemyComp);
	for (auto& s : state) {
		s->InitState();
	}
	curState = STATE_WANDER;
}

void KG::Component::CrawlerStateManager::SetState() {
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

void KG::Component::CrawlerStateManager::Execute(float elapsedTime) {
	SetState();
	state[curState]->Execute(elapsedTime);
}

int KG::Component::CrawlerStateManager::GetCurState() const
{
	return curState;
}
