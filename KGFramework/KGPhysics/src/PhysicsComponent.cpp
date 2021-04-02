#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "PhysicsScene.h"
#include "Scene.h"
void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
	XMFLOAT3 pos = transform->GetPosition();
	SetCollisionBox(pos, XMFLOAT3(1, 1, 1));
	KG::Physics::PhysicsScene::GetInstance()->AddDynamicActor(this);
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	positionProp("Position", this->rigid.position),
	scaleProp("Scale", this->rigid.scale),
	offsetProp("Offset", this->rigid.offset),
	applyProp("Apply", this->apply)
{
}

void KG::Component::DynamicRigidComponent::PostUpdate(float timeElapsed)
{
	if (apply) {
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
		physx::PxVec3 p = actor->getGlobalPose().p;
		transform->SetPosition(p.x, p.y, p.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x, p.y, p.z));
		this->actor->clearForce();
		this->actor->clearTorque();
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
	}
}

void KG::Component::DynamicRigidComponent::Update(float timeElapsed)
{

}

void KG::Component::DynamicRigidComponent::SetCollisionBox(DirectX::XMFLOAT3& position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 offset)
{
	collisionBox.center = position;
	collisionBox.scale = scale;
	collisionBox.offset = offset;
}

void KG::Component::DynamicRigidComponent::SetActor(physx::PxRigidDynamic* actor)
{
	this->actor = actor;
}

void KG::Component::DynamicRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->offsetProp.OnDataLoad(componentElement);
	this->applyProp.OnDataLoad(componentElement);
}

void KG::Component::DynamicRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::DynamicRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->offsetProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
}

bool KG::Component::DynamicRigidComponent::OnDrawGUI()
{
	// static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::)
	// static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
	// static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);

	// if (this->isUsing())
	// 	return false;

	if (ImGui::ComponentHeader<KG::Component::DynamicRigidComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->offsetProp.OnDrawGUI();
			ImGui::TreePop();
		}
		this->applyProp.OnDrawGUI();
		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		auto curr = this->gameObject->GetTransform()->GetGlobalWorldMatrix();
		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
		ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m), reinterpret_cast<const float*>(proj.m), reinterpret_cast<const float*>(curr.m), 1);
	}
	return false;
}
