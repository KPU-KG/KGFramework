#include "pch.h"
#include "ClientEnemyControllerComponent.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "imgui/imgui.h"
#include "Transform.h"

bool KG::Component::CEnemyControllerComponent::RotateToGoal(float elapsedTime)
{
	return false;
}

bool KG::Component::CEnemyControllerComponent::MoveToGoal()
{
	return false;
}

bool KG::Component::CEnemyControllerComponent::Idle(float elapsedTime)
{
	return false;
}

KG::Component::CEnemyControllerComponent::CEnemyControllerComponent()
{

}

void KG::Component::CEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);
	// this->SetNetObjectId(this->server->GetNewObjectId());

	this->transform = this->gameObject->GetTransform();
	// this->center = this->transform->GetWorldPosition();
	this->rigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();
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
		auto* p = KG::Packet::PacketCast<KG::Packet::SC_MOVE_OBJECT>(packet);
		this->rigid->SetPosition(p->position);
		this->rigid->SetRotation(p->rotation);
		return true;
		// break;
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
			ImGui::TextDisabled("Goal : (%f, %f, %f)", goal.x, goal.y, goal.z);
			ImGui::TextDisabled("this->angle : (%f, %f)", this->angle.x, this->angle.y);
		}
	}
	return false;
}

void KG::Component::CEnemyControllerComponentSystem::OnPostUpdate(float elapsedTime)
{
}

void KG::Component::CEnemyControllerComponentSystem::OnPreRender()
{
}
