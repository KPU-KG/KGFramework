#include "pch.h"
#include "ServerProjectileComponent.h"
#include "PhysicsComponent.h"
#include "Transform.h"
#include "KGServer.h"
#include "PhysicsComponent.h"

KG::Component::SProjectileComponent::SProjectileComponent()
{

}

void KG::Component::SProjectileComponent::Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float speed, float damage)
{
	this->rigid->SetPosition(origin);
	this->transform->SetPosition(origin);
	this->transform->SetRotation(direction.x, direction.y, direction.z, 1);

	auto dir = direction;
	auto look = transform->GetWorldLook();
	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	XMFLOAT3 angle;
	XMStoreFloat3(&angle, DirectX::XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), angle.x));
	gameObject->GetTransform()->Rotate(rot);
	rigid->SetRotation(transform->GetRotation());

	this->rigid->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
		// my 위치에 파티클
		DebugNormalMessage("Projectile object collide");
		auto filterMy = my->GetFilterMask();
		auto filterOther = other->GetFilterGroup();
		if (!(filterMy & filterOther)) {
			auto col = other->GetCollisionCallback();
			if (col != nullptr)
				col(other, my);
			this->GetGameObject()->Destroy();
		}
		});
	this->direction = direction;
	this->speed = speed;
	this->damage = damage;
}

void KG::Component::SProjectileComponent::SetTargetPosition(DirectX::XMFLOAT3 pos)
{
	this->targetPos = pos;
	this->isSetTarget = true;
}

void KG::Component::SProjectileComponent::OnCreate(KG::Core::GameObject* obj)
{
	KG::Component::SBaseComponent::OnCreate(obj);
	this->rigid = gameObject->GetComponent<DynamicRigidComponent>();
	this->transform = gameObject->GetTransform();
}

void KG::Component::SProjectileComponent::Update(float elapsedTime)
{
	this->rigid->SetVelocity(this->direction, this->speed);
	auto dir = direction;
	auto look = transform->GetWorldLook();
	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	XMFLOAT3 angle;
	XMStoreFloat3(&angle, DirectX::XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&dir)));

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), angle.x));
	gameObject->GetTransform()->Rotate(rot);
	rigid->SetRotation(transform->GetRotation());

	sendTimer += elapsedTime;
	if (sendInterval <= sendTimer) {
		KG::Packet::SC_MOVE_OBJECT packet;
		packet.position = this->transform->GetWorldPosition();
		auto rot = this->transform->GetRotation();
		packet.rotation = rot;

		this->BroadcastPacket((void*)&packet);

		sendTimer = 0;
	}
}

void KG::Component::SProjectileComponent::OnDestroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	IComponent::OnDestroy();
}

bool KG::Component::SProjectileComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	// 사실상 이건 딱히 필요 없을듯
	return false;
}

// void KG::Component::SProjectileComponent::Destroy()
// {
// 	// if (this->rigid)
// 	// 	rigid->ReleaseActor();
// 
// 	// this->gameObject->Destroy();
// }

bool KG::Component::SProjectileComponent::IsDelete() const
{
	return this->isDelete;
}
