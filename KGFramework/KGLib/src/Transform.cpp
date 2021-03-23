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
	static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);

	if ( ImGui::ComponentHeader< KG::Component::TransformComponent>() )
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
		if ( this->gameObject->GetScene()->GetMainCamera() )
		{
			ImGui::Text("Edit Mode");
			if ( ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE) )
				currentGizmoOperation = ImGuizmo::TRANSLATE;
			//ImGui::SameLine();
			//if ( ImGui::RadioButton("Rotate", currentGizmoOperation == ImGuizmo::ROTATE) )
			//	currentGizmoOperation = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if ( ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE) )
				currentGizmoOperation = ImGuizmo::SCALE;

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
			auto view = this->gameObject->GetScene()->GetMainCameraView();
			auto proj = this->gameObject->GetScene()->GetMainCameraProj();
			auto curr = this->GetGlobalWorldMatrix();
			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);
			curr = Math::Matrix4x4::Transpose(curr);
			float t[3] = { 0,0,0 };
			float r[3] = { 0,0,0 };
			float s[3] = { 0,0,0 };
			ImGuizmo::DecomposeMatrixToComponents((float*)curr.m, t, r, s);
			auto worldPos = this->GetWorldPosition();
			t[0] = worldPos.x;
			t[1] = worldPos.y;
			t[2] = worldPos.z;
			ImGuizmo::RecomposeMatrixFromComponents(t, r, s, (float*)curr.m);
			float delta[16] = {};
			if ( ImGuizmo::Manipulate((float*)view.m, (float*)proj.m, currentGizmoOperation, currentGizmoMode, (float*)curr.m, delta, NULL) )
			{
				flag = true;
				ImGuizmo::DecomposeMatrixToComponents(delta, t, r, s);
				switch ( currentGizmoOperation )
				{
					case ImGuizmo::TRANSLATE:
						this->SetPosition(Math::Vector3::Add(this->position, XMFLOAT3(t)));
						break;
					case ImGuizmo::ROTATE:
						this->SetEulerDegree(Math::Vector3::Add(this->eulerRotation, XMFLOAT3(r)));
						break;
					case ImGuizmo::SCALE:
						this->SetScale(Math::Vector3::Multiply(this->scale, XMFLOAT3(s)));
						break;
					default:
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
