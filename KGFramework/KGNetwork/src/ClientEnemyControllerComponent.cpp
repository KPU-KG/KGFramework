#include "pch.h"
#include "ClientEnemyControllerComponent.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "imgui/imgui.h"
#include "Transform.h"

KG::Component::CEnemyControllerComponent::CEnemyControllerComponent()
{

}

void KG::Component::CEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);

	this->transform = this->gameObject->GetTransform();
	this->anim = this->gameObject->GetComponent<AnimationControllerComponent>();
}

void KG::Component::CEnemyControllerComponent::Update(float elapsedTime)
{
}

bool KG::Component::CEnemyControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch (type)
	{
	case KG::Packet::PacketType::SC_MOVE_OBJECT:
	{
		auto* p = KG::Packet::PacketCast<KG::Packet::SC_MOVE_OBJECT>(packet);
		this->transform->SetPosition(p->position);
		this->transform->SetRotation(p->rotation);
		return true;
	}
	case KG::Packet::PacketType::SC_CHANGE_ANIMATION:
	{
		auto* p = KG::Packet::PacketCast<KG::Packet::SC_CHANGE_ANIMATION>(packet);
		this->anim->ChangeAnimation(p->animId, p->animIndex, p->nextState, p->blendingTime, p->repeat);
		return true;
	}
	}
	return false;
}

bool KG::Component::CEnemyControllerComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::CEnemyControllerComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			if (transform) {
				auto center = transform->GetPosition();
				ImGui::TextDisabled("Center : (%f, %f, %f)", center.x, center.y, center.z);
				auto angle = transform->GetEulerDegree();
				ImGui::TextDisabled("rotation : (%f, %f, %f)", angle.x, angle.y, angle.z);
			}
		}
	}
	return false;
}