
#include "pch.h"

#include "ServerEnemyCrawlerComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IAnimationComponent.h"
#include "ServerCubeAreaRed.h"
#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include "Scene.h"


std::random_device crawlerRd;
std::mt19937 crawlerGen(crawlerRd());

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
	hp = maxHp;
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

	return false;
}



bool KG::Component::SEnemyCrawlerComponent::SetTarget()
{
	// 일단 가장 가까운 플레이어 선택

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
	if (minId != -1) {
		auto comp = this->server->FindNetObject(minId);
		if (comp == nullptr) {
			playerId.erase(minId);
			target = nullptr;
		}
		else
			target = comp;
	}
	return true;
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
}

bool KG::Component::SEnemyCrawlerComponent::SetAttackArea()
{
	if (this->target == nullptr)
		return true;

	if (!this->target->isUsing())
		return true;

	shootAreaCenter = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	shootAreaCenter.y = 0.5;
	std::uniform_real_distribution<float> shootAreaRange(-areaWidth, areaWidth);

	while (!shootArea.empty())
		shootArea.pop();

	for (int i = 0; i < 8; ++i) {
		float x = shootAreaRange(crawlerGen) + shootAreaCenter.x;
		float z = shootAreaRange(crawlerGen) + shootAreaCenter.z;
		shootArea.push(std::make_pair(x, z));
	}

	// Area 생성

	return true;
}

bool KG::Component::SEnemyCrawlerComponent::Shoot(float elapsedTime)
{
	if (!inShootAction)
		inShootAction = true;

	if (this->target == nullptr)
		return true;

	if (attackTimer == 0 && !isInAttackDelay) {
		if (this->anim)
			ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::idle, ANIMSTATE_STOP, 0.1, 1);
		isAttackable = true;
		isInAttackDelay = true;
		if (shootArea.empty()) {
			inShootAction = false;
			return true;
		}
		else {
			auto p = shootArea.front();
			shootArea.pop();
			shootTarget.x = p.first;
			shootTarget.y = 0;
			shootTarget.z = p.second;
		}
	}

	if (isInAttackDelay) {
		attackTimer += elapsedTime;
		if (attackTimer >= attackInterval) {
			isInAttackDelay = false;
			attackTimer = 0;
		}
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

void KG::Component::SEnemyCrawlerComponent::Attack(SGameManagerComponent* gameManager)
{
	if (stateManager->GetCurState() == CrawlerStateManager::STATE_SHOOT_ATTACK) {
		if (inShootAction) {
			if (!isFilledArea) {
				auto presetName = "CubeAreaRed";
				auto presetId = KG::Utill::HashString(presetName);

				auto* scene = this->gameObject->GetScene();
				auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

				KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
				auto tag = KG::Utill::HashString(presetName);
				addObjectPacket.objectTag = tag;
				addObjectPacket.parentTag = 0;
				addObjectPacket.presetId = tag;
				addObjectPacket.position = shootAreaCenter;

				auto id = this->server->GetNewObjectId();
				addObjectPacket.newObjectId = id;
				comp->SetNetObjectId(id);
				this->server->SetServerObject(id, comp);

				auto areaComp = comp->GetGameObject()->GetComponent<SCubeAreaRedComponent>();

				areaComp->Initialize(shootAreaCenter, areaWidth);
				
				this->area = areaComp;

				this->server->SetServerObject(id, areaComp);
				gameManager->BroadcastPacket(&addObjectPacket);
				this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());

				isFilledArea = true;
			}
		}
		else {
			isFilledArea = false;
			if (area)
				area->GetGameObject()->Destroy();
			area = nullptr;
		}

		auto presetName = "Missile";
		auto presetId = KG::Utill::HashString(presetName);

		auto* scene = this->gameObject->GetScene();
		auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

		auto origin = this->transform->GetWorldPosition();
		origin.y += 10;
		auto direction = Math::Vector3::Normalize(Math::Vector3::Subtract(shootTarget, origin));


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
		projectile->SetTargetPosition(shootTarget);

		this->server->SetServerObject(id, projectile);
		gameManager->BroadcastPacket(&addObjectPacket);
		this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());
	}
}

bool KG::Component::CrawlerIdleAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->Idle(elapsedTime);
}

void KG::Component::CrawlerIdleAction::EndAction() {
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetIdleTime(0);
}

bool KG::Component::CrawlerSetTargetAction::Execute(float elapsedTime) {
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetTarget();
}

void KG::Component::CrawlerSetTargetAction::EndAction() {

}

bool KG::Component::CrawlerShootAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->Shoot(elapsedTime);
}

void KG::Component::CrawlerShootAction::EndAction()
{
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::CrawlerSetAreaAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->SetAttackArea();
}

void KG::Component::CrawlerSetAreaAction::EndAction()
{
}


KG::Component::CrawlerShootState::~CrawlerShootState()
{
	for (auto& act : action)
		delete act;
}

void KG::Component::CrawlerShootState::InitState()
{
	action[SHOOT_ACTION_SET_AREA] = new CrawlerSetAreaAction(enemyComp);
	action[SHOOT_ACTION_ATTACK] = new CrawlerShootAction(enemyComp);
}

void KG::Component::CrawlerShootState::Execute(float elapsedTime)
{
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case SHOOT_ACTION_SET_AREA:
			curAction = SHOOT_ACTION_ATTACK;
			break;
		case SHOOT_ACTION_ATTACK:
			curAction = SHOOT_ACTION_SET_AREA;
			isFinished = true;
			break;
		}
	}
}

float KG::Component::CrawlerShootState::GetValue()
{
	if (isFinished)
		return 1.f;
	else
		return 0.f;
}


KG::Component::CrawlerSetTargetState::~CrawlerSetTargetState()
{
	for (auto& act : action)
		delete act;
}

inline void KG::Component::CrawlerSetTargetState::InitState()
{
	action[SET_TARGET_ACTION_IDLE] = new CrawlerIdleAction(enemyComp);
	action[SET_TARGET_ACTION_SET_TARGET] = new CrawlerSetTargetAction(enemyComp);
}

void KG::Component::CrawlerSetTargetState::Execute(float elapsedTime)
{
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case SET_TARGET_ACTION_IDLE:
			curAction = SET_TARGET_ACTION_SET_TARGET;
			break;
		case SET_TARGET_ACTION_SET_TARGET:
			curAction = SET_TARGET_ACTION_IDLE;
			isFinished = true;
			break;
		}
	}
}

float KG::Component::CrawlerSetTargetState::GetValue()
{
	if (isFinished)
		return 1.f;
	else
		return 0.f;
}

KG::Component::CrawlerStateManager::CrawlerStateManager(SEnemyUnitComponent* comp) : enemyComp(comp) { }

KG::Component::CrawlerStateManager::~CrawlerStateManager() {
	for (auto& s : state) {
		delete s;
	}
}

void KG::Component::CrawlerStateManager::Init() {
	state[STATE_SET_TARGET] = new CrawlerSetTargetState(enemyComp);
	state[STATE_SHOOT_ATTACK] = new CrawlerShootState(enemyComp);
	for (auto& s : state) {
		s->InitState();
	}
	curState = STATE_SET_TARGET;
}

void KG::Component::CrawlerStateManager::SetState() {
	auto v = state[curState]->GetValue();
	if (v >= 1) {
		switch (curState) {
		case STATE_SET_TARGET:
			// 패턴 추가되면 랜덤으로 선택하도록 수정
			dynamic_cast<CrawlerSetTargetState*>(state[curState])->isFinished = false;
			curState = STATE_SHOOT_ATTACK;
			break;
		case STATE_SHOOT_ATTACK:
			dynamic_cast<CrawlerShootState*>(state[curState])->isFinished = false;
			curState = STATE_SET_TARGET;
			break;
		}
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