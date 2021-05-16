#include "pch.h"
#include "ServerProjectileComponent.h"
#include "PhysicsComponent.h"
#include "Transform.h"
#include "KGServer.h"

KG::Component::SProjectileComponent::SProjectileComponent()
{

}

void KG::Component::SProjectileComponent::Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float speed, float damage)
{
	this->rigid->SetPosition(origin);
	this->rigid->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
		// my ��ġ�� ��ƼŬ
		DebugNormalMessage("Projectile object collide");
		this->Destroy();
		});
	this->direction = direction;
	this->speed = speed;
	this->damage = damage;
}

void KG::Component::SProjectileComponent::OnCreate(KG::Core::GameObject* obj)
{
	KG::Component::SBaseComponent::OnCreate(obj);
	this->rigid = gameObject->GetComponent<DynamicRigidComponent>();
	this->transform = gameObject->GetTransform();
}

void KG::Component::SProjectileComponent::Update(float elapsedTime)
{
	// this->rigid->
	// this->rigid->SetVelocity()
	this->rigid->AddForce(this->direction, speed * elapsedTime);
}

bool KG::Component::SProjectileComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	// ��ǻ� �̰� ���� �ʿ� ������
	return false;
}

void KG::Component::SProjectileComponent::Destroy()
{
	// this->server->LockWorld();
	if (this->rigid)
		rigid->ReleaseActor();
	// this->server->UnlockWorld();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	this->gameObject->Destroy();
}

bool KG::Component::SProjectileComponent::IsDelete() const
{
	return this->isDelete;
}
