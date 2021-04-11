#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "PhysicsScene.h"
#include "Scene.h"
#include "MathHelper.h"
#include <unordered_map>

void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
	KG::Physics::PhysicsScene::GetInstance()->AddDynamicActor(this);
	SetupFiltering(this->actor, KG::Physics::FilterGroup::eBOX, 0);
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	positionProp("Position", this->collisionBox.center),
	scaleProp("Scale", this->collisionBox.scale),
	applyProp("Apply", this->apply),
	showProp(
		"Show", this->show,
		{
			{SHOW_COLLISION_BOX::NONE, "None"},
			{SHOW_COLLISION_BOX::BOX, "Box"},
			{SHOW_COLLISION_BOX::GRID, "Grid"}
		}, false)
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

void KG::Component::DynamicRigidComponent::Move(DirectX::XMFLOAT3 direction, float speed) {
	this->actor->setLinearVelocity(physx::PxVec3(direction.x, direction.y, direction.z) * speed * 100);
}

void KG::Component::DynamicRigidComponent::SetActor(physx::PxRigidDynamic* actor)
{
	this->actor = actor;
	// actor->
}

// physx::PxSimulationEventCallback::onContact

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

		switch (show) {
		case SHOW_COLLISION_BOX::NONE:
			break;
		case SHOW_COLLISION_BOX::BOX:
		{
			auto objectPos = this->gameObject->GetTransform()->GetPosition();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&objectPos)));
			mat = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}
			break;
		case SHOW_COLLISION_BOX::GRID:
			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

			auto objectPos = this->gameObject->GetTransform()->GetPosition();
			XMFLOAT4X4 mats[6];

			// front
			auto front = objectPos;
			front.z += collisionBox.scale.z / 2;
			DirectX::XMStoreFloat4x4(&mats[0], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&front)));
			mats[0] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90), 0, 0) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[0]);

			// back
			auto back = objectPos;
			back.z -= collisionBox.scale.z / 2;
			DirectX::XMStoreFloat4x4(&mats[1], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&back)));
			mats[1] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90), 0, 0) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[1]);

			// right
			auto right = objectPos;
			right.x += collisionBox.scale.x / 2;
			DirectX::XMStoreFloat4x4(&mats[2], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&right)));
			mats[2] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(90)) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[2]);

			// left
			auto left = objectPos;
			back.x -= collisionBox.scale.x / 2;
			DirectX::XMStoreFloat4x4(&mats[3], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&left)));
			mats[3] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(90)) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[3]);

			// up
			auto up = objectPos;
			up.y += collisionBox.scale.y / 2;
			DirectX::XMStoreFloat4x4(&mats[4], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&up)));
			mats[4] = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[4]);

			// down
			auto down = objectPos;
			back.y -= collisionBox.scale.y / 2;
			DirectX::XMStoreFloat4x4(&mats[5], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&up)));
			mats[5] = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[5]);

			for (int i = 0; i < 6; ++i) {
				ImGuizmo::DrawGrid(reinterpret_cast<const float*>(view.m),
					reinterpret_cast<const float*>(proj.m),
					reinterpret_cast<const float*>(mats[i].m),
					1);
			}
			break;
		}

	}
	return false;
}

void KG::Component::StaticRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
	KG::Physics::PhysicsScene::GetInstance()->AddStaticActor(this);
	SetupFiltering(this->actor, KG::Physics::FilterGroup::eBUILDING, 0);
}

KG::Component::StaticRigidComponent::StaticRigidComponent()
	:
	positionProp("Position", this->collisionBox.center),
	scaleProp("Scale", this->collisionBox.scale),
	showProp(
		"Show", this->show,
		{
			{SHOW_COLLISION_BOX::NONE, "None"},
			{SHOW_COLLISION_BOX::BOX, "Box"},
			{SHOW_COLLISION_BOX::GRID, "Grid"}
		},false)
{
}

void KG::Component::StaticRigidComponent::PostUpdate(float timeElapsed)
{
	XMFLOAT3 p = transform->GetPosition();
	actor->setGlobalPose(physx::PxTransform(p.x + collisionBox.center.x, p.y + collisionBox.center.y, p.z + collisionBox.center.z));
}

void KG::Component::StaticRigidComponent::Update(float timeElapsed)
{

}

void KG::Component::StaticRigidComponent::SetActor(physx::PxRigidStatic* actor)
{
	this->actor = actor;
}

void KG::Component::StaticRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->showProp.OnDataLoad(componentElement);
}

void KG::Component::StaticRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::StaticRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->showProp.OnDataSave(componentElement);
}

bool KG::Component::StaticRigidComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::StaticRigidComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			this->positionProp.OnDrawGUI();
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
			this->showProp.OnDrawGUI();
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			ImGui::TreePop();
			this->showProp.OnDrawGUI();
		}


		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		switch (show) {
		case SHOW_COLLISION_BOX::NONE:
			break;
		case SHOW_COLLISION_BOX::BOX:
		{
			auto objectPos = this->gameObject->GetTransform()->GetPosition();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&objectPos)));
			mat = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}
		break;
		case SHOW_COLLISION_BOX::GRID:
			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

			auto objectPos = this->gameObject->GetTransform()->GetPosition();
			XMFLOAT4X4 mats[6];

			// front
			auto front = objectPos;
			front.z += collisionBox.scale.z / 2;
			DirectX::XMStoreFloat4x4(&mats[0], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&front)));
			mats[0] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90), 0, 0) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[0]);

			// back
			auto back = objectPos;
			back.z -= collisionBox.scale.z / 2;
			DirectX::XMStoreFloat4x4(&mats[1], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&back)));
			mats[1] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90), 0, 0) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[1]);

			// right
			auto right = objectPos;
			right.x += collisionBox.scale.x / 2;
			DirectX::XMStoreFloat4x4(&mats[2], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&right)));
			mats[2] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(90)) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[2]);

			// left
			auto left = objectPos;
			back.x -= collisionBox.scale.x / 2;
			DirectX::XMStoreFloat4x4(&mats[3], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&left)));
			mats[3] = Math::Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(0, 0, XMConvertToRadians(90)) * XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[3]);

			// up
			auto up = objectPos;
			up.y += collisionBox.scale.y / 2;
			DirectX::XMStoreFloat4x4(&mats[4], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&up)));
			mats[4] = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[4]);

			// down
			auto down = objectPos;
			back.y -= collisionBox.scale.y / 2;
			DirectX::XMStoreFloat4x4(&mats[5], DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&up)));
			mats[5] = Math::Matrix4x4::Multiply(XMMatrixScalingFromVector(
				XMLoadFloat3(&collisionBox.scale) / 2) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)), mats[5]);

			for (int i = 0; i < 6; ++i) {
				ImGuizmo::DrawGrid(reinterpret_cast<const float*>(view.m),
					reinterpret_cast<const float*>(proj.m),
					reinterpret_cast<const float*>(mats[i].m),
					1);
			}
			break;
		}
	}
	return false;
}

void KG::Component::SetupFiltering(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
{
	physx::PxFilterData filterData;
	filterData.word0 = filterGroup; // word0 = own ID
	filterData.word1 = filterMask;	// word1 = ID mask to filter pairs that trigger a contact callback;
	const physx::PxU32 numShapes = actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape * [numShapes];
	actor->getShapes(shapes, numShapes);
	for (physx::PxU32 i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(filterData);
	}
	delete[] shapes;
}
