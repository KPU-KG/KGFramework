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
	KG::Physics::PhysicsScene::GetInstance()->AddDynamicActor(this);
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	positionProp("Position", this->collisionBox.center),
	scaleProp("Scale", this->collisionBox.scale),
	offsetProp("Offset", this->collisionBox.offset),
	applyProp("Apply", this->apply)
{
}

void KG::Component::DynamicRigidComponent::PostUpdate(float timeElapsed)
{
	if (apply) {
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		physx::PxVec3 p = actor->getGlobalPose().p;
		transform->SetPosition(p.x, p.y, p.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x, p.y, p.z));
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
	if (ImGui::ComponentHeader<KG::Component::DynamicRigidComponent>()) {
			ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			ImGui::TextDisabled("Position (%.3f, %.3f, %.3f)", collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->offsetProp.OnDrawGUI();
			ImGui::TreePop();
		}

		// 큐브 그리기
		this->applyProp.OnDrawGUI();
		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		// auto curr = this->gameObject->GetTransform()->GetGlobalWorldMatrix();		// world matrix
		XMFLOAT3 pos = this->gameObject->GetTransform()->GetPosition();
		XMFLOAT4 rot = this->gameObject->GetTransform()->GetRotation();
		// XMQUATERNION
		XMFLOAT4X4 mat;
		XMStoreFloat4x4(&mat, XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&pos)), XMMatrixRotationQuaternion(XMLoadFloat4(&rot))));
		XMFLOAT4X4 colMat;
		XMStoreFloat4x4(&colMat, XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale))));
		XMStoreFloat4x4(&colMat, XMMatrixMultiply(XMLoadFloat4x4(&colMat), XMLoadFloat4x4(&mat)));

		// 근데 생각해보니까 게임 오브젝트의 스케일은 거의 항상 111인데 이걸 가져오면 안되잖아???
		// 그에에ㅔㅔㄱ..
			// actor
		// XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&mat), DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale))));
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&mat), DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center))));
		// XMStoreFloat4x4(&curr, XMMatrixMultiply(XMLoadFloat4x4(&curr), DirectX::XMLoadFloat4x4(&mat)));
		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m),
			reinterpret_cast<const float*>(proj.m),
			reinterpret_cast<const float*>(colMat.m),
			1);
		// this->applyProp.OnDrawGUI();
		// auto view = this->gameObject->GetScene()->GetMainCameraView();
		// auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		// auto curr = this->gameObject->GetTransform()->GetGlobalWorldMatrix();		// world matrix
		// DirectX::XMFLOAT4X4 mat;
		// // actor
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&curr), DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center))));
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&mat), DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale))));
		// 
		// view = Math::Matrix4x4::Transpose(view);
		// proj = Math::Matrix4x4::Transpose(proj);
		// ImGuiIO& io = ImGui::GetIO();
		// ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		// ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m), 
		// 	reinterpret_cast<const float*>(proj.m), 
		// 	reinterpret_cast<const float*>(mat.m), 
		// 	1);
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
	offsetProp("Offset", this->collisionBox.offset),
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
		transform->SetPosition(p.x, p.y, p.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x, p.y, p.z));
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
	this->offsetProp.OnDataLoad(componentElement);
	this->applyProp.OnDataLoad(componentElement);
}

void KG::Component::StaticRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::StaticRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->offsetProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
}

bool KG::Component::StaticRigidComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::StaticRigidComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			ImGui::TextDisabled("Position (%.3f, %.3f, %.3f)", collisionBox.center.x, collisionBox.center.y, collisionBox.center.z);
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->offsetProp.OnDrawGUI();
			ImGui::TreePop();
		}
		this->applyProp.OnDrawGUI();
		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();
		// auto curr = this->gameObject->GetTransform()->GetGlobalWorldMatrix();		// world matrix
		XMFLOAT3 pos = this->gameObject->GetTransform()->GetPosition();
		XMFLOAT4 rot = this->gameObject->GetTransform()->GetRotation();
		// XMQUATERNION
		XMFLOAT4X4 mat;
		XMStoreFloat4x4(&mat, XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&pos)), XMMatrixRotationQuaternion(XMLoadFloat4(&rot))));
		XMFLOAT4X4 colMat;
		XMStoreFloat4x4(&colMat, XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale))));
		XMStoreFloat4x4(&colMat, XMMatrixMultiply(XMLoadFloat4x4(&colMat), XMLoadFloat4x4(&mat)));

		// 근데 생각해보니까 게임 오브젝트의 스케일은 거의 항상 111인데 이걸 가져오면 안되잖아???
		// 그에에ㅔㅔㄱ..
			// actor
		// XMStoreFloat4x4(&mat, DirectX::XMMatrixIdentity());
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&mat), DirectX::XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale))));
		// XMStoreFloat4x4(&mat, XMMatrixMultiply(XMLoadFloat4x4(&mat), DirectX::XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center))));
		// XMStoreFloat4x4(&curr, XMMatrixMultiply(XMLoadFloat4x4(&curr), DirectX::XMLoadFloat4x4(&mat)));
		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);
		ImGuiIO& io = ImGui::GetIO();
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::DrawCubes(reinterpret_cast<const float*>(view.m),
			reinterpret_cast<const float*>(proj.m),
			reinterpret_cast<const float*>(colMat.m),
			1);
	}
	return false;
}
