#include "pch.h"
#include "Transform.h"
#include "GameObject.h"
#include "Scene.h"
using namespace KG::Component;

KG::Component::TransformComponent::TransformComponent()
	:
	positionProp("Position", this->position),
	rotationEulerProp("Rotation", this->eulerRotation),
	rotationQautProp("Rotation_Q", this->rotation),
	scaleProp("Scale", this->scale),
	worldPositionProp("WPosition", this->worldPosition)
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
	static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);

	if ( ImGui::ComponentHeader< KG::Component::TransformComponent>() )
	{
		bool flag = false;
		this->eulerRotation = this->GetEulerDegree();
		this->worldPosition = this->GetWorldPosition();
		flag |= this->positionProp.OnDrawGUI();
		flag |= this->worldPositionProp.OnDrawGUI();

		XMFLOAT3 prev = this->eulerRotation;

		if ( this->rotationEulerProp.OnDrawGUI() )
		{
			if ( prev.x != this->eulerRotation.x )
			{
				this->RotateAxis(this->GetRight(), this->eulerRotation.x - prev.x);
			}
			else if ( prev.y != this->eulerRotation.y )
			{
				this->RotateAxis(this->GetUp(), this->eulerRotation.y - prev.y);
			}
			else if ( prev.z != this->eulerRotation.z )
			{
				this->RotateAxis(this->GetLook(), this->eulerRotation.z - prev.z);
			}
			//this->SetEulerDegree(this->eulerRotation);
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
		if ( this->gameObject->GetScene()->GetMainCamera() )
		{
			ImGui::Text("Edit Mode");
			if ( ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE) )
				currentGizmoOperation = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if ( ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE) )
				currentGizmoOperation = ImGuizmo::SCALE;
			ImGui::SameLine();
			if ( ImGui::RadioButton("Off", currentGizmoOperation == 0) )
				currentGizmoOperation = (ImGuizmo::OPERATION)0;

			if ( ImGui::RadioButton("RotateX", currentGizmoOperation == ImGuizmo::ROTATE_X) )
				currentGizmoOperation = ImGuizmo::ROTATE_X;
			ImGui::SameLine();
			if ( ImGui::RadioButton("RotateY", currentGizmoOperation == ImGuizmo::ROTATE_Y) )
				currentGizmoOperation = ImGuizmo::ROTATE_Y;
			ImGui::SameLine();
			if ( ImGui::RadioButton("RotateZ", currentGizmoOperation == ImGuizmo::ROTATE_Z) )
				currentGizmoOperation = ImGuizmo::ROTATE_Z;



			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			auto view = this->gameObject->GetScene()->GetMainCameraView();
			auto proj = this->gameObject->GetScene()->GetMainCameraProj();
			auto curr = this->GetGlobalWorldMatrix();
			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);
			float t[3] = { 0,0,0 }; float r[3] = { 0,0,0 }; float s[3] = { 0,0,0 };
			DirectX::XMFLOAT4X4 delta;

			if ( currentGizmoOperation != 0 && ImGuizmo::Manipulate((float*)view.m, (float*)proj.m, currentGizmoOperation, currentGizmoMode, (float*)curr.m, NULL, NULL) )
			{
				if ( this->GetParent() )
				{
					auto parentWorld = this->GetParent()->GetGlobalWorldMatrix();
					auto parentWorldMat = XMLoadFloat4x4(&parentWorld);
					auto currentWorldMat = XMLoadFloat4x4(&curr);
					auto currentLocal = XMMatrixMultiply(currentWorldMat, XMMatrixInverse(NULL, parentWorldMat));
					XMStoreFloat4x4(&delta, currentLocal);
				}
				flag = true;
				ImGuizmo::DecomposeMatrixToComponents((float*)delta.m, t, r, s);
				auto euler = this->GetEulerDegree();
				switch ( currentGizmoOperation )
				{
					case ImGuizmo::TRANSLATE:
						this->SetPosition(XMFLOAT3(t));
						break;
					case ImGuizmo::ROTATE_X:
						this->RotateAxis(this->GetRight(), this->eulerRotation.x -  r[0]);
						break;
					case ImGuizmo::ROTATE_Y:
						this->RotateAxis(this->GetUp(), r[1] - this->eulerRotation.y);
						break;
					case ImGuizmo::ROTATE_Z:
						this->RotateAxis(this->GetLook(), r[2] - this->eulerRotation.z);
						break;
					case ImGuizmo::SCALE:
						this->SetScale(XMFLOAT3(s));
						break;
				}
			}
		}
		if ( flag )
		{
			this->TurnOnLocalDirtyFlag();
		}
	}
	return false;
}
