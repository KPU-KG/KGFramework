#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"

void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	rigidTypeProp("RigidType", this->rigidType,
		{
			{RigidType::DynamicRigid, "DynamicRigid"},
			{RigidType::StaticRigid, "StaticRigid"},
		}),
	positionProp("Position", this->rigid.position),
	widthProp("Width", this->rigid.width),
	heightProp("Height", this->rigid.height),
	depthProp("Depth", this->rigid.depth),
	offsetProp("Offset", this->rigid.offset)
{
}

void KG::Component::DynamicRigidComponent::PostUpdate(float timeElapsed)
{
	physx::PxVec3 p = actor->getGlobalPose().p;
	transform->SetPosition(p.x, p.y, p.z);

	// rotation 쪽은 좀더 알아보고..
}

void KG::Component::DynamicRigidComponent::SetCollisionBox(DirectX::XMFLOAT3& position, float w, float h, float d, DirectX::XMFLOAT3 offset)
{
	collisionBox.center = position;
	collisionBox.width = w;
	collisionBox.height = h;
	collisionBox.depth = d;
	collisionBox.offset = offset;
}

void KG::Component::DynamicRigidComponent::SetActor(physx::PxRigidDynamic* actor)
{
	this->actor = actor;
}

void KG::Component::DynamicRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->rigidTypeProp.OnDataLoad(componentElement);
	this->positionProp.OnDataLoad(componentElement);
	this->widthProp.OnDataLoad(componentElement);
	this->heightProp.OnDataLoad(componentElement);
	this->depthProp.OnDataLoad(componentElement);
	this->offsetProp.OnDataLoad(componentElement);

	// is kinetic
}

void KG::Component::DynamicRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::DynamicRigidComponent);
	this->rigidTypeProp.OnDataSave(componentElement);
	this->positionProp.OnDataSave(componentElement);
	this->widthProp.OnDataSave(componentElement);
	this->heightProp.OnDataSave(componentElement);
	this->depthProp.OnDataSave(componentElement);
	this->offsetProp.OnDataSave(componentElement);
}

bool KG::Component::DynamicRigidComponent::OnDrawGUI()
{
	// 여긴 나중에 물어보고 함쉬다
	// if (ImGui::ComponentHeader<DynamicRigidComponent()>)
	// {
	// 	bool flag = false;
	// 	this->rigidTypeProp.OnDrawGUI();
	// 	// flag |=this->
	// }

	return false;
}
