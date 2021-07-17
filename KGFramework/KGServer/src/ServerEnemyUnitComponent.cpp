#include "pch.h"
#include "ServerEnemyUnitComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IAnimationComponent.h"

#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include "Scene.h"


KG::Component::SEnemyUnitComponent::SEnemyUnitComponent()
{

}

void KG::Component::SEnemyUnitComponent::SetCenter(DirectX::XMFLOAT3 center) {
	this->center = center;
}

void KG::Component::SEnemyUnitComponent::SetSpeed(float speed) {
	this->speed = speed;
}

void KG::Component::SEnemyUnitComponent::SetIdleInterval(float interval) {
	this->idleInterval = interval;
}

void KG::Component::SEnemyUnitComponent::SetRotateInterval(float interval) {
	this->rotateInterval = interval;
}

void KG::Component::SEnemyUnitComponent::SetWanderRange(float range) {
	this->range = range;
}

void KG::Component::SEnemyUnitComponent::SetPosition(DirectX::XMFLOAT3 position)
{
	this->rigid->SetPosition(position);
}

void KG::Component::SEnemyUnitComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	this->transform = this->gameObject->GetTransform();
	this->center = this->transform->GetWorldPosition();
	this->rigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();
	if (this->rigid)
		this->rigid->SetRaycastCallback(raycastCallback);
	this->anim = this->gameObject->GetComponent<IAnimationControllerComponent>();
}

void KG::Component::SEnemyUnitComponent::Update(float elapsedTime)
{

}

bool KG::Component::SEnemyUnitComponent::OnDrawGUI()
{

	return false;
}

void KG::Component::SEnemyUnitComponent::SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback) {
	this->raycastCallback = callback;
}

void KG::Component::SEnemyUnitComponent::HitBullet() {

}

bool KG::Component::SEnemyUnitComponent::IsDead() const
{
	return isDead;
}

bool KG::Component::SEnemyUnitComponent::IsDelete() const
{
	return destroyInterval <= destroyTimer;
}


void KG::Component::SEnemyUnitComponent::RegisterPlayerId(KG::Server::NET_OBJECT_ID id)
{
	if (playerId.count(id) == 0)
		playerId.insert(id);
}

void KG::Component::SEnemyUnitComponent::DeregisterPlayerId(KG::Server::NET_OBJECT_ID id)
{
	if (playerId.count(id) > 0)
		playerId.erase(id);
}

void KG::Component::SEnemyUnitComponent::Destroy()
{

}

bool KG::Component::SEnemyUnitComponent::IsAttackable() const
{
	return isAttackable;
}

void KG::Component::SEnemyUnitComponent::PostAttack()
{
	isAttackable = false;
}

void KG::Component::SEnemyUnitComponent::Attack(SGameManagerComponent* gameManager)
{

}


bool KG::Component::SEnemyUnitComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}


KG::Component::StateManager::~StateManager() {

}

void KG::Component::StateManager::Init() {

}

void KG::Component::StateManager::SetState() {

}

void KG::Component::StateManager::Execute(float elapsedTime) {

}
