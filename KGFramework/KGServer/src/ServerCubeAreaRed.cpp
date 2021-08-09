#include "pch.h"
#include "ServerCubeAreaRed.h"
#include "Transform.h"

KG::Component::SCubeAreaRedComponent::SCubeAreaRedComponent()
{
}

void KG::Component::SCubeAreaRedComponent::Initialize(DirectX::XMFLOAT3 center, float width, float interval)
{
	DebugNormalMessage("Create Red Area");
	this->transform->SetPosition(center);
	this->width = width;
	this->transform->SetScale(width, 0.01, width);
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
		scaleP.scale = XMFLOAT3{ width, 0.01, width };
		this->BroadcastPacket((void*)&scaleP);

		KG::Packet::SC_MOVE_OBJECT moveP;
		moveP.position = this->transform->GetWorldPosition();
		auto rot = this->transform->GetRotation();
		moveP.rotation = rot;

		this->BroadcastPacket((void*)&moveP);


		// �����ϸ� ���� Ŀ���� �� ����
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
	// ��� ��
	return false;
}

bool KG::Component::SCubeAreaRedComponent::IsDelete() const
{
	return this->isDelete;
}
