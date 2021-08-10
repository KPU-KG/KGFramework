#include "pch.h"
#include "ServerCubeAreaRed.h"
#include "Transform.h"

KG::Component::SCubeAreaRedComponent::SCubeAreaRedComponent()
{
}

void KG::Component::SCubeAreaRedComponent::Initialize(DirectX::XMFLOAT3 center, DirectX::XMFLOAT3 scale, float interval)
{
	DebugNormalMessage("Create Red Area");
	this->transform->SetPosition(center);
	this->scale = scale;
	this->transform->SetScale(scale);
}

void KG::Component::SCubeAreaRedComponent::SetRotation(DirectX::XMFLOAT4 rot)
{
	this->transform->SetRotation(rot);
}

void KG::Component::SCubeAreaRedComponent::OnCreate(KG::Core::GameObject* obj)
{
	KG::Component::SBaseComponent::OnCreate(obj);
	this->transform = gameObject->GetTransform();
}

void KG::Component::SCubeAreaRedComponent::Update(float elapsedTime)
{
	sendTimer += elapsedTime;
	if (sendInterval <= sendTimer) {
		KG::Packet::SC_SCALE_OBJECT scaleP = {};
		scaleP.scale = this->scale;
		this->BroadcastPacket((void*)&scaleP);

		KG::Packet::SC_MOVE_OBJECT moveP;
		moveP.position = this->transform->GetWorldPosition();
		auto rot = this->transform->GetRotation();
		moveP.rotation = rot;

		this->BroadcastPacket((void*)&moveP);


		// 가능하면 점점 커지는 것 적용
		sendTimer = 0;
	}
}

void KG::Component::SCubeAreaRedComponent::OnDestroy()
{
	KG::Packet::SC_REMOVE_OBJECT removeObjectPacket = {};
	this->BroadcastPacket((void*)&removeObjectPacket);
	IComponent::OnDestroy();
}

bool KG::Component::SCubeAreaRedComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	// 없어도 됨
	return false;
}

bool KG::Component::SCubeAreaRedComponent::IsDelete() const
{
	return this->isDelete;
}
