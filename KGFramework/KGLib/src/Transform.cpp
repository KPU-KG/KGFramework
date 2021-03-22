#include "pch.h"
#include "Transform.h"
using namespace KG::Component;

KG::Component::TransformComponent::TransformComponent()
	:
	positionProp("Position", this->position),
	rotationEulerProp("Rotation", this->eulerRotation),
	rotationQautProp("Rotation_Q", this->rotation),
	scaleProp("Scale", this->scale)
{

}

void KG::Component::TransformComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->rotationQautProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
}

void KG::Component::TransformComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::TransformComponent);
	this->positionProp.OnDataSave(componentElement);
	this->rotationQautProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
}

bool KG::Component::TransformComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader< KG::Component::TransformComponent>())
	{
		bool flag = false;
		this->eulerRotation = this->GetEulerDegree();
		flag |= this->positionProp.OnDrawGUI();

		if ( this->rotationEulerProp.OnDrawGUI() )
		{
			this->SetEulerDegree(this->eulerRotation);
		}
		if ( ImGui::TreeNode("Quaternion") )
		{
			if ( this->rotationQautProp.OnDrawGUI() )
			{
				this->rotation.x = Math::CycleValue(this->rotation.x, -1.0f, 1.0f);
				this->rotation.y = Math::CycleValue(this->rotation.y, -1.0f, 1.0f);
				this->rotation.z = Math::CycleValue(this->rotation.z, -1.0f, 1.0f);
				this->rotation.w = Math::CycleValue(this->rotation.w, -1.0f, 1.0f);
				flag = true;
				//this->eulerRotation = this->GetEulerDegree();
			}
			ImGui::TreePop();
		}
		flag |= this->scaleProp.OnDrawGUI();
		if ( flag )
		{
			this->TurnOnLocalDirtyFlag();
		}
	}
	return false;
}
