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

	// goal.x = mechGoalRange(mechGen) * range + center.x;
	// goal.z = mechGoalRange(mechGen) * range + center.z;
	// 
	// auto pos = transform->GetWorldPosition();
	// distance = std::sqrt(std::pow((goal.x - pos.x), 2) + std::pow((goal.y - pos.y), 2));
	// 
	// auto spd = speed;
	// if (this->stateManager->GetCurState() == MechStateManager::STATE_TRACE)
	// 	spd += traceStateSpeed;
	// 
	// arriveTime = distance / spd;
	// moveTime = 0;
	// 
	// direction = Math::Vector3::Subtract(goal, transform->GetWorldPosition());
	// direction.y = 0;
	// XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));
	// 
	auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };
	auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };

	XMFLOAT2 angle;

	XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	XMFLOAT2 crs;
	XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	if (crs.x >= 0)
		angle.x *= -1;

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, angle.x, 0));
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

	auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };
	auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };

	XMFLOAT2 angle;

	XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	XMFLOAT2 crs;
	XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	if (crs.x >= 0)
		angle.x *= -1;

	DirectX::XMFLOAT4 rot;
	XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, angle.x, 0));
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
