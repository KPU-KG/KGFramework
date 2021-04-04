#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "PhysicsScene.h"
#include "Scene.h"
#include "MathHelper.h"

void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
	KG::Physics::PhysicsScene::GetInstance()->AddDynamicActor(this);
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	positionProp("Position", this->collisionBox.center),
	scaleProp("Scale", this->collisionBox.scale),
	applyProp("Apply", this->apply)
{
}

void KG::Component::DynamicRigidComponent::PostUpdate(float timeElapsed)
{
	if (apply) {
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		physx::PxVec3 p = actor->getGlobalPose().p;
		transform->SetPosition(p.x - collisionBox.center.x, p.y - collisionBox.center.y, p.z - collisionBox.center.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x + collisionBox.center.x, p.y + collisionBox.center.y, p.z + collisionBox.center.z));
		this->actor->clearForce();
		this->actor->clearTorque();
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
	}
}

void KG::Component::DynamicRigidComponent::Update(float timeElapsed)
{

}

void KG::Component::DynamicRigidComponent::SetActor(physx::PxRigidDynamic* actor)
{
	this->actor = actor;
}

void KG::Component::DynamicRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->applyProp.OnDataLoad(componentElement);
}

void KG::Component::DynamicRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::DynamicRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
}

bool KG::Component::DynamicRigidComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::DynamicRigidComponent>()) {
			ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			this->positionProp.OnDrawGUI();
			// ImGui::TextDisabled("Position (%.3f, %.3f, %.3f)", collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			ImGui::TreePop();
		}

		// 큐브 그리기
		this->applyProp.OnDrawGUI();
		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		// auto curr = this->gameObject->GetTransform()->GetGlobalWorldMatrix();		// world matrix
		auto objectPos = this->gameObject->GetTransform()->GetPosition();
		
		XMFLOAT4X4 mat;
		DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&objectPos)));
		mat = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
			XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mat);
		// curr = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
		// 	XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)),
		// 	curr);
		


		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m),
			reinterpret_cast<const float*>(proj.m),
			reinterpret_cast<const float*>(mat.m),
			1);

	}
	return false;
}

void KG::Component::StaticRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
	KG::Physics::PhysicsScene::GetInstance()->AddStaticActor(this);
}

KG::Component::StaticRigidComponent::StaticRigidComponent()
	:
	positionProp("Position", this->collisionBox.center),
	scaleProp("Scale", this->collisionBox.scale),
	applyProp("Apply", this->apply)
{
}

void KG::Component::StaticRigidComponent::PostUpdate(float timeElapsed)
{

}

void KG::Component::StaticRigidComponent::Update(float timeElapsed)
{
	if (apply) {
		physx::PxVec3 p = actor->getGlobalPose().p;
		transform->SetPosition(p.x + collisionBox.center.x, p.y + collisionBox.center.y, p.z + collisionBox.center.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x + collisionBox.center.x, p.y + collisionBox.center.y, p.z + collisionBox.center.z));
	}
}

void KG::Component::StaticRigidComponent::SetActor(physx::PxRigidStatic* actor)
{
	this->actor = actor;
}

void KG::Component::StaticRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->applyProp.OnDataLoad(componentElement);
}

void KG::Component::StaticRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::StaticRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
}

bool KG::Component::StaticRigidComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::StaticRigidComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			this->positionProp.OnDrawGUI();
			// ImGui::TextDisabled("Position (%.3f, %.3f, %.3f)", collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			ImGui::TreePop();
		}
		this->applyProp.OnDrawGUI();
		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		auto objectPos = this->gameObject->GetTransform()->GetPosition();

		XMFLOAT4X4 mat;
		DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&objectPos)));
		mat = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
			XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mat);
		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m),
			reinterpret_cast<const float*>(proj.m),
			reinterpret_cast<const float*>(mat.m),
			1);
	}
	return false;
}
