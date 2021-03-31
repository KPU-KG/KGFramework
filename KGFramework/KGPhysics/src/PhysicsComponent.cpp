#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "PhysicsScene.h"

void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
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
		physx::PxVec3 p = actor->getGlobalPose().p;
		transform->SetPosition(p.x, p.y, p.z);
	}
	else {
		XMFLOAT3 p = transform->GetPosition();
		actor->setGlobalPose(physx::PxTransform(p.x, p.y, p.z));
		this->actor->clearForce();
		this->actor->clearTorque();
	}

	// rotation 쪽은 좀더 알아보고..
}

void KG::Component::DynamicRigidComponent::Update(float timeElapsed)
{
	// auto trans = gameObject->GetComponent<KG::Component::TransformComponent>();
	// using namespace KG::Input;
	// auto input = InputManager::GetInputManager();
	// float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
	// speed *= speedValue;
	// if (ImGui::IsAnyItemFocused())
	// {
	// 	return;
	// }
	// if (input->IsTouching('W'))
	// {
	// 	trans->Translate(trans->GetLook() * speed * elapsedTime);
	// }
	// if (input->IsTouching('A'))
	// {
	// 	trans->Translate(trans->GetRight() * speed * elapsedTime * -1);
	// }
	// if (input->IsTouching('S'))
	// {
	// 	trans->Translate(trans->GetLook() * speed * elapsedTime * -1);
	// }
	// if (input->IsTouching('D'))
	// {
	// 	trans->Translate(trans->GetRight() * speed * elapsedTime);
	// }
	// if (input->IsTouching('E'))
	// {
	// 	trans->Translate(trans->GetUp() * speed * elapsedTime);
	// }
	// if (input->IsTouching('Q'))
	// {
	// 	trans->Translate(trans->GetUp() * speed * elapsedTime * -1);
	// }
	// 
	// if (input->IsTouching('O'))
	// {
	// 	this->speedValue -= 0.5f;
	// }
	// if (input->IsTouching('P'))
	// {
	// 	this->speedValue += 0.5f;
	// }
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

	if (ImGui::ComponentHeader<KG::Component::DynamicRigidComponent>()) {
		if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->offsetProp.OnDrawGUI();
			ImGui::TreePop();
		}
		this->applyProp.OnDrawGUI();
	}
	return false;
}
