
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

	this->rigid->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
		auto filterMy = my->GetFilterMask();
		auto filterOther = other->GetFilterGroup();
		// if (this->IsCharging()) {
		if (filterOther & static_cast<uint32_t>(FilterGroup::ePLAYER)) {
			auto col = other->GetCollisionCallback();
			if (col != nullptr)
				col(other, my);
		}
		// }

		if (!(filterMy & filterOther)) {
			auto col = other->GetCollisionCallback();
			if (col != nullptr)
				col(other, my);
		}
		});
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
		for (auto e = this->areaEvent.begin(); e != this->areaEvent.end();) {
			if (e->area == nullptr) {
				e = this->areaEvent.erase(e);
				continue;
			}
			e->timer -= elapsedTime;
			
			if (e->timer <= 0) {
				e->area->GetGameObject()->Destroy();
				e = this->areaEvent.erase(e);
				continue;
			}

			e++;
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

void KG::Component::SEnemyCrawlerComponent::HitBullet() {
	this->hp -= 1;

	KG::Packet::SC_ENEMY_HP hp;
	hp.percentage = float(this->hp) / float(maxHp);
	this->BroadcastPacket(&hp);
}

bool KG::Component::SEnemyCrawlerComponent::IsCharging() const
{
	return this->isCharging;
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
	if (target == nullptr)
		return false;

	return true;
}

void KG::Component::SEnemyCrawlerComponent::Destroy()
{
	for (auto iter : this->areaEvent) {
		if (iter.area != nullptr)
			iter.area->GetGameObject()->Destroy();
	}
	this->areaEvent.clear();

	if (this->area != nullptr) {
		this->area->GetGameObject()->Destroy();
		this->area = nullptr;
	}

	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	gameObject->Destroy();
}

bool KG::Component::SEnemyCrawlerComponent::SetAttackArea()
{
	if (this->target == nullptr)
		return true;

	if (!this->target->isUsing())
		return true;

	shootAreaCenter = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	shootAreaCenter.y = 0.1;
	std::uniform_real_distribution<float> shootAreaRange(-areaWidth, areaWidth);

	while (!shootArea.empty())
		shootArea.pop();

	for (int i = 0; i < 8; ++i) {
		float x = shootAreaRange(crawlerGen) + shootAreaCenter.x;
		float z = shootAreaRange(crawlerGen) + shootAreaCenter.z;
		shootArea.push(std::make_pair(x, z));
	}
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

bool KG::Component::SEnemyCrawlerComponent::Rotate(float elapsedTime)
{
	if (this->target == nullptr)
		return true;

	auto look = this->transform->GetWorldLook();
	auto dir = Math::Vector3::Subtract(this->target->GetGameObject()->GetTransform()->GetWorldPosition(), this->transform->GetWorldPosition());

	look.y = 0;
	dir.y = 0;
	
	XMFLOAT3 angle;
	XMStoreFloat3(&angle, XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	float amount = min(abs(angle.x), this->rotateSpeed * elapsedTime);

	if (crs.y >= 0) {
		amount *= -1;
	}

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), amount));
	gameObject->GetTransform()->Rotate(rot);
	rigid->SetRotation(transform->GetRotation());

	if (amount == abs(angle.x)) {
		chargeOrigin = this->transform->GetWorldPosition();
		chargeTarget = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
		chargeDist = sqrt(GetDistance2FromEnemy(chargeTarget));
		moveDist = 0;
		prevPosition = this->chargeOrigin;
		return true;
	}
	return false;
}

bool KG::Component::SEnemyCrawlerComponent::Charging(float elapsedTime)
{
	if (this->target == nullptr)
		return true;

	if (!isCharging) {
		isCharging = true;
		isAttackable = true;
	}

	chargingTimer += elapsedTime;
	if (chargingTimer >= chargingInterval) {
		chargingTimer = 0;
		return true;
	}

	return false;
}

bool KG::Component::SEnemyCrawlerComponent::ChargeAttack(float elapsedTime)
{
	if (this->target == nullptr)
		return true;

	if (!changedAnimation) {
		ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	auto dir = Math::Vector3::Subtract(this->chargeTarget, this->chargeOrigin);
	XMStoreFloat3(&dir, XMVector3Normalize(XMLoadFloat3(&dir)));
	this->rigid->SetVelocity(dir, this->chargeSpeed);

	moveDist += sqrt(GetDistance2FromEnemy(this->prevPosition));
	this->prevPosition = this->transform->GetWorldPosition();
	if (moveDist >= chargeDist) {
		this->rigid->SetVelocity(XMFLOAT3{ 0,0,0 }, 0);
		isCharging = false;
		return true;
	}

	return false;
}

bool KG::Component::SEnemyCrawlerComponent::ChargeDelay(float elapsedTIme)
{
	if (isFilledArea) {
		if (this->area != nullptr) {
			this->area->GetGameObject()->Destroy();
			this->area = nullptr;
		}
		isFilledArea = false;
	}

	if (this->target == nullptr)
		return true;

	if (!changedAnimation) {
		ChangeAnimation(KG::Utill::HashString("crawler.fbx"_id), KG::Component::CrawlerAnimIndex::idle, ANIMSTATE_PLAYING, 0.1f, -1);
	}

	chargeDelayTimer += elapsedTIme;

	if (chargeDelayTimer >= chargeDelay) {
		chargeDelayTimer = 0;
		return true;
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

				areaComp->Initialize(shootAreaCenter, XMFLOAT3{ areaWidth, 0.001, areaWidth });
				
				this->area = areaComp;

				this->server->SetServerObject(id, areaComp);
				gameManager->BroadcastPacket(&addObjectPacket);
				this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());

				isFilledArea = true;
			}
		}
		else {
			isFilledArea = false;
			if (area) {
				AreaEvent e;
				e.area = this->area;
				e.timer = 2.6f;
				this->areaEvent.emplace_back(e);
			}
			area = nullptr;
		}

		auto presetName = "CrawlerMissile";
		auto presetId = KG::Utill::HashString(presetName);

		auto* scene = this->gameObject->GetScene();
		auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

		auto pos = this->transform->GetWorldPosition();
		pos.y += 3;

		KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
		auto tag = KG::Utill::HashString(presetName);
		addObjectPacket.objectTag = tag;
		addObjectPacket.parentTag = 0;
		addObjectPacket.presetId = tag;
		addObjectPacket.position = pos;

		auto id = this->server->GetNewObjectId();
		addObjectPacket.newObjectId = id;
		comp->SetNetObjectId(id);
		this->server->SetServerObject(id, comp);

		auto missile = comp->GetGameObject()->GetComponent<SCrawlerMissileComponent>();
		missile->Initialize(pos, shootTarget);

		this->server->SetServerObject(id, missile);
		gameManager->BroadcastPacket(&addObjectPacket);
		this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());
	}
	else if (stateManager->GetCurState() == CrawlerStateManager::STATE_CHARGE_ATTACK) {
		if (!isFilledArea) {
			auto presetName = "CubeAreaRed";
			auto presetId = KG::Utill::HashString(presetName);

			auto* scene = this->gameObject->GetScene();
			auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));

			// x축 2 z축 타겟과의 거리?

			float dist = sqrt(GetDistance2FromEnemy(this->chargeTarget));

			XMFLOAT3 areaCenter;
			XMStoreFloat3(&areaCenter, Math::Vector3::XMVectorLerp(XMLoadFloat3(&this->chargeOrigin), XMLoadFloat3(&this->chargeTarget), 0.5));

			areaCenter.y = 0.1;

			XMFLOAT3 areaScale{ 3, 0.001, dist};

			KG::Packet::SC_ADD_OBJECT addObjectPacket = {};
			auto tag = KG::Utill::HashString(presetName);
			addObjectPacket.objectTag = tag;
			addObjectPacket.parentTag = 0;
			addObjectPacket.presetId = tag;
			addObjectPacket.position = areaCenter;

			auto id = this->server->GetNewObjectId();
			addObjectPacket.newObjectId = id;
			comp->SetNetObjectId(id);
			this->server->SetServerObject(id, comp);

			auto areaComp = comp->GetGameObject()->GetComponent<SCubeAreaRedComponent>();

			areaComp->Initialize(areaCenter, areaScale);
			areaComp->SetRotation(this->transform->GetRotation());

			this->area = areaComp;

			this->server->SetServerObject(id, areaComp);
			gameManager->BroadcastPacket(&addObjectPacket);
			this->GetGameObject()->GetTransform()->GetParent()->AddChild(comp->GetGameObject()->GetTransform());

			isFilledArea = true;
		}
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

bool KG::Component::CrawlerRotateAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->Rotate(elapsedTime);
}

void KG::Component::CrawlerRotateAction::EndAction()
{
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::CrawlerChargingAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->Charging(elapsedTime);
}

void KG::Component::CrawlerChargingAction::EndAction()
{
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}

bool KG::Component::CrawlerChargeAttackAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ChargeAttack(elapsedTime);
}

void KG::Component::CrawlerChargeAttackAction::EndAction()
{
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
}


bool KG::Component::CrawlerChargeDelayAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ChargeDelay(elapsedTime);
}

void KG::Component::CrawlerChargeDelayAction::EndAction()
{
	dynamic_cast<SEnemyCrawlerComponent*>(enemyComp)->ReadyNextAnimation(false);
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


KG::Component::CrawlerChargeState::~CrawlerChargeState()
{
	for (auto& act : action)
		delete act;
}

void KG::Component::CrawlerChargeState::InitState()
{
	action[CHARGE_ACTION_ROTATE] = new CrawlerRotateAction(enemyComp);
	action[CHARGE_ACTION_CHARGING] = new CrawlerChargingAction(enemyComp);
	action[CHARGE_ACTION_ATTACK] = new CrawlerChargeAttackAction(enemyComp);
	action[CHARGE_ACTION_DELAY] = new CrawlerChargeDelayAction(enemyComp);
}

void KG::Component::CrawlerChargeState::Execute(float elapsedTime)
{
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		switch (curAction) {
		case CHARGE_ACTION_ROTATE:
			curAction = CHARGE_ACTION_CHARGING;
			break;
		case CHARGE_ACTION_CHARGING:
			curAction = CHARGE_ACTION_ATTACK;
			break;
		case CHARGE_ACTION_ATTACK:
			curAction = CHARGE_ACTION_DELAY;
			break;
		case CHARGE_ACTION_DELAY:
			curAction = CHARGE_ACTION_ROTATE;
			isFinished = true;
			break;
		}
	}
}

float KG::Component::CrawlerChargeState::GetValue()
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
	state[STATE_CHARGE_ATTACK] = new CrawlerChargeState(enemyComp);
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
			{
				std::uniform_int_distribution<int> nextState(0, 1);

				switch (nextState(crawlerGen)) {
				case 0:
					curState = STATE_CHARGE_ATTACK;
					break;
				case 1:
					curState = STATE_SHOOT_ATTACK;
					break;
				}
			}
			break;
		case STATE_SHOOT_ATTACK:
			dynamic_cast<CrawlerShootState*>(state[curState])->isFinished = false;
			curState = STATE_SET_TARGET;
			break;
		case STATE_CHARGE_ATTACK:
			dynamic_cast<CrawlerChargeState*>(state[curState])->isFinished = false;
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
