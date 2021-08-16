#include "pch.h"
#include "ServerEnemyTurretComponent.h"
#include "KGServer.h"
#include "Transform.h"
#include "IPhysicsScene.h"
#include "Scene.h"
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
			inRangePlayer.push_back(id);
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
		this->inAttack = false;
		this->curAttackCount = 0;
		this->tempAttackTimer = 0;
		this->attackTimer = 0;
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

	crs = Math::Vector3::Normalize(crs);

	float amount = min(abs(angle.x), this->rotateSpeed * elapsedTime);

	if (angle.x < 0)
		amount *= -1;

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), amount));
	this->gunTransform->Rotate(rot);

	if (this->inAttack) {
		this->tempAttackTimer += elapsedTime;
		return true;
	}

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
		}
		return true;
	}

	if (isInAttackDelay) {
		attackTimer += elapsedTime + this->tempAttackTimer;
		this->tempAttackTimer = 0;
		if (attackTimer >= attackInterval) {
			isInAttackDelay = false;
			attackTimer = 0;
		}
		if (this->inAttack)
			return true;
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
	if (this->target == nullptr) {
		return;
	}

	if (!this->target->isUsing()) {
		this->target = nullptr;
		return;
	}

	this->curAttackCount++;

	auto presetName = "Missile";
	auto presetId = KG::Utill::HashString(presetName);

	auto* scene = this->gameObject->GetScene();
	auto* comp = static_cast<SBaseComponent*>(scene->CallNetworkCreator(KG::Utill::HashString(presetName)));


	auto targetPos = this->target->GetGameObject()->GetTransform()->GetWorldPosition();
	targetPos.y += 0.3;
	auto origin = this->transform->GetWorldPosition();
	origin.y += 3.5;
	
	// auto direction = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, origin));
	auto dir = this->gunTransform->GetWorldLook();
	dir.y *= 0.5;
	auto direction = Math::Vector3::Normalize(dir);

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
}

KG::Component::SEnemyTurretComponent::SEnemyTurretComponent()
{
}

void KG::Component::SEnemyTurretComponent::OnCreate(KG::Core::GameObject* obj)
{
	SEnemyUnitComponent::OnCreate(obj);
	this->gunTransform = this->gameObject->FindChildObject("TurretGun"_id)->GetTransform();
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

void KG::Component::SEnemyTurretComponent::Awake()
{
	this->isAttacked = true;
	this->attackRange = 180;
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
