#include "pch.h"
#include "ClientEnemyControllerComponent.h"
#include "PhysicsComponent.h"
#include "IAnimationComponent.h"
#include "IRender2DComponent.h"
#include "imgui/imgui.h"
#include "Transform.h"
#include "Scene.h"
#include "IParticleEmitterComponent.h"

KG::Component::CEnemyControllerComponent::CEnemyControllerComponent()
{

}

void KG::Component::CEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	CBaseComponent::OnCreate(obj);

	this->transform = this->gameObject->GetTransform();
	this->anim = this->gameObject->GetComponent<IAnimationControllerComponent>();
    this->hpBar = this->gameObject->FindChildObject("HPBAR"_id)->GetComponent<KG::Component::IRenderSpriteComponent>();
    this->gameObject->GetScene()->GetComponentProvider()->AddComponentToObject(KG::Component::ComponentID<IParticleEmitterComponent>::id(), this->gameObject);
    this->particle = this->gameObject->GetComponent<IParticleEmitterComponent>();

    auto* pe = this->gameObject->FindChildObject("Pelvis"_id);
    if(!pe)
        pe = this->gameObject->FindChildObject("joint23"_id);
	if (pe)
		this->pelvis = pe->GetComponent<TransformComponent>();

	auto* gun = this->gameObject->FindChildObject("TurretGun"_id);
	if (gun) {
		this->gunTransform = gun->GetComponent<TransformComponent>();
	}
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
		if (this->gunTransform) {
			this->gunTransform->SetRotation(p->rotation);
		}
		else {
			this->transform->SetRotation(p->rotation);
		}
		return true;
	}
	case KG::Packet::PacketType::SC_CHANGE_ANIMATION:
	{
		auto* p = KG::Packet::PacketCast<KG::Packet::SC_CHANGE_ANIMATION>(packet);
		this->anim->ChangeAnimation(p->animId, p->animIndex, p->nextState, p->blendingTime, p->repeat);
		return true;
	}
	case KG::Packet::PacketType::SC_REMOVE_OBJECT:
	{
		this->gameObject->Destroy();
        return true;
	}
    case KG::Packet::PacketType::SC_ENEMY_HP:
    {
        auto* p = KG::Packet::PacketCast<KG::Packet::SC_ENEMY_HP>(packet);
        prevHP = this->hpBar->progress.value;
        this->hpBar->progress.value = p->percentage;
        if(prevHP > this->hpBar->progress.value)
        {
			if (this->pelvis)
				this->particle->EmitParticle("EXPSpark"_id, this->pelvis->GetWorldPosition());
			else if (this->gunTransform)
				this->particle->EmitParticle("EXPSpark"_id, this->gunTransform->GetWorldPosition());
        }
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
        ImGui::Text("netId = %d", this->networkObjectId);
	}
	return false;
}