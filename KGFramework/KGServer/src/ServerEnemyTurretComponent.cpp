#include "ServerEnemyTurretComponent.h"
#include "KGServer.h"
#include "Transform.h"
#include "IPhysicsScene.h"
#include <random>

std::random_device turretRd;
std::mt19937 turretGen(turretRd());

bool KG::Component::SEnemyTurretComponent::Idle(float elapsedTime)
{
	idleTimer += elapsedTime;
	if (idleInterval <= idleTimer) {
		idleTimer = 0;
		return true;
	}
	return false;
}

bool KG::Component::SEnemyTurretComponent::SetTarget()
{
	if (this->inAttack) {
		if (this->curAttackCount <= this->attackCount)
			return true;
	}

	std::vector<KG::Server::NET_OBJECT_ID> inRangePlayer;
	KG::Server::NET_OBJECT_ID minId = -1;
	for (auto& id : playerId) {
		auto* comp = this->server->FindNetObject(id);
		auto targetPos = comp->GetGameObject()->GetTransform()->GetWorldPosition();
		float d = GetDistance2FromPos(targetPos);
		if (d < this->attackRange * this->attackRange) {
			auto pos = this->transform->GetWorldPosition();
			auto dir = Math::Vector3::Subtract(targetPos, pos);

			auto c = this->rigid->GetScene()->QueryRaycast(pos, dir, this->attackRange, rigid->GetId());

			if (c != nullptr) {
				if (c->GetFilterGroup() & static_cast<uint32_t>(FilterGroup::ePLAYER))
					inRangePlayer.push_back(id);
			}
		}
	}

	if (inRangePlayer.size() == 0) {
		return false;
	}

	std::uniform_real_distribution<float> targetRange(1, inRangePlayer.size());
	this->target = this->server->FindNetObject(inRangePlayer[targetRange(turretGen) - 1]);

	return true;
}

bool KG::Component::SEnemyTurretComponent::Rotate(float elapsedTime)
{
	if (this->target == nullptr) {
		return true;
	}
	if (!this->target->isUsing()) {
		this->target = nullptr;
		return true;
	}

	auto g = this->target->GetGameObject()->GetTransform()->GetWorldPosition();

	auto look = this->gunTransform->GetWorldLook();
	auto dir = Math::Vector3::Subtract(g, this->transform->GetWorldPosition());
	look.y = 0;
	dir.y = 0;
	look = Math::Vector3::Normalize(look);
	dir = Math::Vector3::Normalize(dir);

	XMFLOAT3 angle;
	XMStoreFloat3(&angle, XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	float amount = min(abs(angle.x), this->rotateSpeed * elapsedTime);

	if (angle.x < 0)
		amount *= -1;

	this->gunTransform->RotateAxis(crs, amount);
	// rigid->SetRotation(this->gunTransform->GetRotation());

	if (abs(amount) >= abs(angle.x)) {
		return true;
	}

	return false;


}

bool KG::Component::SEnemyTurretComponent::AttackTarget(float elapsedTime)
{
	if (this->target == nullptr)
		return true;

	this->inAttack = true;

	if (attackTimer == 0 && !isInAttackDelay) {
		isAttackable = true;
		isInAttackDelay = true;
		if (this->curAttackCount >= this->attackCount) {
			this->curAttackCount = 0;
			this->inAttack = false;
			return true;
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

float KG::Component::SEnemyTurretComponent::GetDistance2FromPos(DirectX::XMFLOAT3 pos) const
{
	auto myPos = this->transform->GetWorldPosition();
	float goalDistance = (pos.x - myPos.x) * (pos.x - myPos.x) + (pos.z - myPos.z) * (pos.z - myPos.z);
	return goalDistance;
}

bool KG::Component::SEnemyTurretComponent::IsInAttack() const
{
	return this->inAttack;
}

void KG::Component::SEnemyTurretComponent::Attack(SGameManagerComponent* gameManager)
{










}

KG::Component::SEnemyTurretComponent::SEnemyTurretComponent()
{
}

void KG::Component::SEnemyTurretComponent::OnCreate(KG::Core::GameObject* obj)
{
	SEnemyUnitComponent::OnCreate(obj);
	this->stateManager = new TurretStateManager(this);
	stateManager->Init();
	hp = maxHp;
}

void KG::Component::SEnemyTurretComponent::Update(float elapsedTime)
{
	if (hp <= 0) {
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
			p.rotation = this->gunTransform->GetRotation();
			this->BroadcastPacket(&p);
		}
		sendTimer = 0;
	}
}

bool KG::Component::SEnemyTurretComponent::OnDrawGUI()
{
	return false;
}

void KG::Component::SEnemyTurretComponent::HitBullet()
{
	this->hp -= 1;

	KG::Packet::SC_ENEMY_HP hp;
	hp.percentage = float(this->hp) / float(maxHp);
	this->BroadcastPacket(&hp);

	this->isAttacked;
}

void KG::Component::SEnemyTurretComponent::Destroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	gameObject->Destroy();
}

KG::Component::TurretStateManager::~TurretStateManager()
{
	for (auto& s : state) {
		delete s;
	}
}

void KG::Component::TurretStateManager::Init()
{
	state[STATE_ATTACK] = new TurretAttackState(enemyComp);
	for (auto& s : state) {
		s->InitState();
	}
	curState = STATE_ATTACK;
}

void KG::Component::TurretStateManager::SetState()
{

}

void KG::Component::TurretStateManager::Execute(float elapsedTime)
{
	SetState();
	state[curState]->Execute(elapsedTime);
}

int KG::Component::TurretStateManager::GetCurState() const
{
	return this->curState;
}

KG::Component::TurretAttackState::~TurretAttackState()
{
	for (auto& act : action)
		delete act;
}

void KG::Component::TurretAttackState::InitState()
{
	action[ATTACK_ACTION_IDLE] = new TurretIdleAction(enemyComp);
	action[ATTACK_ACTION_SET_TARGET] = new TurretSetTargetAction(enemyComp);
	action[ATTACK_ACTION_ROTATE] = new TurretRotateAction(enemyComp);
	action[ATTACK_ACTION_ATTACK] = new TurretAttackAction(enemyComp);
}

void KG::Component::TurretAttackState::Execute(float elapsedTime)
{
	bool endAction = action[curAction]->Execute(elapsedTime);
	if (endAction) {
		action[curAction]->EndAction();
		switch (curAction) {
		case ATTACK_ACTION_IDLE:
			curAction = ATTACK_ACTION_SET_TARGET;
			break;
		case ATTACK_ACTION_SET_TARGET:
			curAction = ATTACK_ACTION_ROTATE;
			break;
		case ATTACK_ACTION_ROTATE:
			curAction = ATTACK_ACTION_ATTACK;
			break;
		case ATTACK_ACTION_ATTACK:
			if (dynamic_cast<SEnemyTurretComponent*>(enemyComp)->IsInAttack())
				curAction = ATTACK_ACTION_SET_TARGET;
			else
				curAction = ATTACK_ACTION_IDLE;
			break;
		}
	}
}

float KG::Component::TurretAttackState::GetValue()
{
	return 0.0f;
}

bool KG::Component::TurretIdleAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyTurretComponent*>(enemyComp)->Idle(elapsedTime);
}

void KG::Component::TurretIdleAction::EndAction()
{
}

bool KG::Component::TurretSetTargetAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyTurretComponent*>(enemyComp)->SetTarget();
}

void KG::Component::TurretSetTargetAction::EndAction()
{
}

bool KG::Component::TurretRotateAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyTurretComponent*>(enemyComp)->Rotate(elapsedTime);
}

void KG::Component::TurretRotateAction::EndAction()
{
}

bool KG::Component::TurretAttackAction::Execute(float elapsedTime)
{
	return dynamic_cast<SEnemyTurretComponent*>(enemyComp)->AttackTarget(elapsedTime);
}

void KG::Component::TurretAttackAction::EndAction()
{
}
