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
	// this->transform->SetRotation(direction.x, direction.y, direction.z, 1);

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
	return false;
}

bool KG::Component::SProjectileComponent::IsDelete() const
{
	return this->isDelete;
}

KG::Component::SCrawlerMissileComponent::SCrawlerMissileComponent()
{
}

void KG::Component::SCrawlerMissileComponent::Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 target)
{
	this->rigid->SetPosition(origin);
	this->transform->SetPosition(origin);
	this->targetPos = target;

	this->isCurved = false;
	this->speed = 30;

	this->curveHeight = 20;
	// 고점 설정
	XMStoreFloat3(&this->topPosition, XMVectorLerp(XMLoadFloat3(&origin), XMLoadFloat3(&target), (2.f / 3.f)));
	this->topPosition.y = this->curveHeight;
	// this->topPosition.y = 10;
	// 방향 설정
	auto look = transform->GetWorldLook();
	this->direction = Math::Vector3::Normalize(Math::Vector3::Subtract(this->topPosition, origin));

	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&this->direction)));

	XMFLOAT3 angle;
	XMStoreFloat3(&angle, DirectX::XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&this->direction)));

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), angle.x));
	gameObject->GetTransform()->Rotate(rot);
	rigid->SetRotation(transform->GetRotation());

	// 충돌 콜백함수 설정
	this->rigid->SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
		auto filterMy = my->GetFilterMask();
		auto filterOther = other->GetFilterGroup();
		if (!(filterMy & filterOther)) {
			auto col = other->GetCollisionCallback();
			if (col != nullptr)
				col(other, my);
			this->GetGameObject()->Destroy();
		}
		});
}

void KG::Component::SCrawlerMissileComponent::OnCreate(KG::Core::GameObject* obj)
{
	KG::Component::SBaseComponent::OnCreate(obj);
	this->rigid = gameObject->GetComponent<DynamicRigidComponent>();
	this->transform = gameObject->GetTransform();
}

void KG::Component::SCrawlerMissileComponent::Update(float elapsedTime)
{
	if (isCurved) {
		this->direction = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, this->transform->GetWorldPosition()));
	}
	else {
		if (this->transform->GetWorldPosition().y >= this->curveHeight) {
			isCurved = true;
			this->direction = Math::Vector3::Normalize(Math::Vector3::Subtract(targetPos, this->transform->GetWorldPosition()));
		}
		else
			this->direction = Math::Vector3::Normalize(Math::Vector3::Subtract(this->topPosition, this->transform->GetWorldPosition()));
	}

	this->rigid->SetVelocity(this->direction, this->speed);

	auto look = transform->GetWorldLook();
	XMFLOAT3 crs;
	XMStoreFloat3(&crs, XMVector3Cross(XMLoadFloat3(&look), XMLoadFloat3(&this->direction)));

	if (crs.x == 0 && crs.y == 0 && crs.z == 0) {

	}
	else {
		XMFLOAT3 angle;
		XMStoreFloat3(&angle, DirectX::XMVector3AngleBetweenVectors(XMLoadFloat3(&look), XMLoadFloat3(&this->direction)));

		DirectX::XMFLOAT4 rot;
		XMStoreFloat4(&rot, XMQuaternionRotationAxis(XMLoadFloat3(&crs), angle.x));
		gameObject->GetTransform()->Rotate(rot);
		rigid->SetRotation(transform->GetRotation());
	}

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

void KG::Component::SCrawlerMissileComponent::OnDestroy()
{
	if (rigid)
		rigid->ReleaseActor();
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	IComponent::OnDestroy();
}

bool KG::Component::SCrawlerMissileComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

bool KG::Component::SCrawlerMissileComponent::IsDelete() const
{
	return isDelete;
}
