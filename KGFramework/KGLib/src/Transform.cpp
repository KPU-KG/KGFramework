#include "pch.h"
#include "Transform.h"
#include "GameObject.h"
#include "Scene.h"
using namespace KG::Component;

void KG::Component::TransformComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	//position = XMFLOAT3( 0, 0, 0 );
	//rotation = XMFLOAT4( 0, 0, 0, 1 ); //사원수
	//scale = XMFLOAT3( 1, 1, 1 );
}

KG::Component::TransformComponent::TransformComponent()
	:
	globalWorldMatrix(KG::Math::Matrix4x4::Identity()),
	localWorldMatrix(KG::Math::Matrix4x4::Identity()),
	positionProp("Position", this->position),
	rotationEulerProp("Rotation", this->eulerRotation),
	rotationQautProp("Rotation_Q", this->rotation),
	scaleProp("Scale", this->scale),
    worldPositionProp("WPosition", this->worldPosition)
{
}

XMFLOAT3 KG::Component::TransformComponent::GetPosition() const
{
    return this->position;
}

XMFLOAT3 KG::Component::TransformComponent::GetWorldPosition() const
{
	const auto& worldMatrix = this->GetGlobalWorldMatrix();
	return XMFLOAT3(worldMatrix._41 / worldMatrix._44, worldMatrix._42 / worldMatrix._44, worldMatrix._43 / worldMatrix._44);
}

void KG::Component::TransformComponent::SetPosition(const XMFLOAT3& position)
{
	this->position = position;
	TurnOnLocalDirtyFlag();
}

void XM_CALLCONV KG::Component::TransformComponent::SetPosition(const FXMVECTOR& position)
{
	XMStoreFloat3(&this->position, position);
	TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::SetPosition(float x, float y, float z)
{
	this->SetPosition(XMFLOAT3(x, y, z));
}

//Quaternion

XMFLOAT4 KG::Component::TransformComponent::GetRotation() const
{
	return this->rotation;
}

void XM_CALLCONV KG::Component::TransformComponent::SetRotation(const FXMVECTOR& rotation)
{
	XMStoreFloat4(&this->rotation, rotation);
	TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::SetRotation(const XMFLOAT4& rotation)
{
	this->rotation = rotation;
	TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::SetRotation(float x, float y, float z, float w)
{
	this->SetRotation(XMFLOAT4(x, y, z, w));
}

//EulerAngle

XMFLOAT3 KG::Component::TransformComponent::GetEulerRadian() const
{
	return KG::Math::Quaternion::ToEuler(this->rotation);
}

void KG::Component::TransformComponent::SetEulerRadian(const XMFLOAT3& angle)
{
	auto angleQuat = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&angle));
	this->SetRotation(angleQuat);
}

void KG::Component::TransformComponent::SetEulerRadian(float x, float y, float z)
{
	this->SetEulerRadian(XMFLOAT3(x, y, z));
}

XMFLOAT3 KG::Component::TransformComponent::GetEulerDegree() const
{
	auto rot = KG::Math::Quaternion::ToEuler(this->rotation);
	rot.x = XMConvertToDegrees(rot.x);
	rot.y = XMConvertToDegrees(rot.y);
	rot.z = XMConvertToDegrees(rot.z);
	return rot;
}

void KG::Component::TransformComponent::SetEulerDegree(const XMFLOAT3& angle)
{
	this->SetEulerDegree(angle.x, angle.y, angle.z);
}

void KG::Component::TransformComponent::SetEulerDegree(float x, float y, float z)
{
	this->SetEulerRadian(XMFLOAT3(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z)));
}

//Scale

XMFLOAT3 KG::Component::TransformComponent::GetScale() const
{
	return this->scale;
}

void XM_CALLCONV KG::Component::TransformComponent::SetScale(const FXMVECTOR& scale)
{
	XMStoreFloat3(&this->scale, scale);
	TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::SetScale(const XMFLOAT3& scale)
{
	this->scale = scale;
	TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::SetScale(float x, float y, float z)
{
	this->SetScale(XMFLOAT3(x, y, z));
}

void KG::Component::TransformComponent::Translate(const XMFLOAT3& move)
{
	using namespace KG::Math;
	this->SetPosition(Vector3::Add(this->position, move));
}

void KG::Component::TransformComponent::Translate(float x, float y, float z)
{
	using namespace KG::Math;
	this->SetPosition(Vector3::Add(this->position, XMFLOAT3(x, y, z)));
}

void KG::Component::TransformComponent::TurnOnLocalDirtyFlag()
{
	this->isDirtyLocal = true;
	TurnOnGlobalDirtyFlag();
}

void KG::Component::TransformComponent::TurnOnGlobalDirtyFlag(bool onTree)
{
	this->isDirtyGlobal = true;
	if ( !onTree )
	{
		this->FunctionChild(
			[](TransformComponent* ptr)
			{
				ptr->TurnOnGlobalDirtyFlag(true);
			}
		);
	}
}

void XM_CALLCONV KG::Component::TransformComponent::Rotate(const FXMVECTOR& quaternion)
{
	auto srcQuatVector = XMLoadFloat4(&this->rotation);
	auto result = XMQuaternionMultiply(srcQuatVector, quaternion);
	XMStoreFloat4(&this->rotation, result);
	this->TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::Rotate(const XMFLOAT4& quaternion)
{
	this->Rotate(XMLoadFloat4(&quaternion));
}

void KG::Component::TransformComponent::RotateAxis(const XMFLOAT3& axis, float angle)
{
	this->Rotate(XMQuaternionRotationAxis(XMLoadFloat3(&axis), XMConvertToRadians(angle)));
}

void KG::Component::TransformComponent::RotateEuler(const XMFLOAT3& euler)
{
	//this->Rotate( XMQuaternionRotationRollPitchYawFromVector( XMLoadFloat3( &euler ) ) );
	this->Rotate(KG::Math::Quaternion::FromEuler(euler));
}

void KG::Component::TransformComponent::RotateEuler(float x, float y, float z)
{
	//this->Rotate(XMQuaternionRotationRollPitchYaw(XMConvertToRadians(x), XMConvertToRadians(y), XMConvertToRadians(z)));
	this->Rotate(KG::Math::Quaternion::FromEuler(XMFLOAT3(x, y, z)));
}

void KG::Component::TransformComponent::Update(float elapsedTime)
{
	this->GetGlobalWorldMatrix();
}

const XMFLOAT4X4& KG::Component::TransformComponent::GetLocalWorldMatrix() const
{
	if ( this->isDirtyLocal && !isUseRawMatrix )
	{
		auto rotMat = XMMatrixRotationQuaternion(XMLoadFloat4(&this->rotation));
		auto scaleMat = XMMatrixScalingFromVector(XMLoadFloat3(&this->scale));
		auto tralationMat = XMMatrixTranslationFromVector(XMLoadFloat3(&this->position));

		//최적화 필요
		XMStoreFloat4x4(&this->localWorldMatrix,
			XMMatrixAffineTransformation(XMLoadFloat3(&this->scale), XMVectorZero(), XMLoadFloat4(&this->rotation), XMLoadFloat3(&this->position))
		);
		//XMStoreFloat4x4( &this->localWorldMatrix, scaleMat * rotMat * tralationMat );

		this->isDirtyLocal = false;
	}
	return this->localWorldMatrix;
}

const XMFLOAT4X4& KG::Component::TransformComponent::GetGlobalWorldMatrix() const
{
	if ( this->isDirtyGlobal )
	{
		if ( this->GetParent() )
		{
			auto parentMatrix = XMLoadFloat4x4(&this->GetParent()->GetGlobalWorldMatrix());
			auto thisMatrix = XMLoadFloat4x4(&this->GetLocalWorldMatrix());
			XMStoreFloat4x4(&this->globalWorldMatrix, thisMatrix * parentMatrix);
		}
		else
		{
			auto thisMatrix = XMLoadFloat4x4(&this->GetLocalWorldMatrix());
			XMStoreFloat4x4(&this->globalWorldMatrix, thisMatrix);
		}
		this->isDirtyGlobal = false;
		//DebugNormalMessage( "GlobalWorldMatrixChange" );
	}
	return this->globalWorldMatrix;
}

XMFLOAT3 KG::Component::TransformComponent::GetLook() const
{
	if ( isUseRawMatrix )
	{
		return XMFLOAT3(
			this->localWorldMatrix._31,
			this->localWorldMatrix._32,
			this->localWorldMatrix._33
		);
	}
	else
	{
		auto p = XMLoadFloat3(&KG::Math::look);
		auto q = XMLoadFloat4(&this->rotation);
		auto r = XMVector3Rotate(p, q);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(r));
		return result;
	}
}

XMFLOAT3 KG::Component::TransformComponent::GetUp() const
{
	if ( isUseRawMatrix )
	{
		return XMFLOAT3(
			this->localWorldMatrix._21,
			this->localWorldMatrix._22,
			this->localWorldMatrix._23
		);
	}
	else
	{
		auto p = XMLoadFloat3(&KG::Math::up);
		auto q = XMLoadFloat4(&this->rotation);
		auto r = XMVector3Rotate(p, q);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(r));
		return result;
	}
}

XMFLOAT3 KG::Component::TransformComponent::GetRight() const
{
	if ( isUseRawMatrix )
	{
		return XMFLOAT3(
			this->localWorldMatrix._11,
			this->localWorldMatrix._12,
			this->localWorldMatrix._13
		);
	}
	else
	{
		auto p = XMLoadFloat3(&KG::Math::right);
		auto q = XMLoadFloat4(&this->rotation);
		auto r = XMVector3Rotate(p, q);
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(r));
		return result;
	}
}

XMFLOAT3 KG::Component::TransformComponent::GlobalTransformNormal(const XMFLOAT3& normal) const
{
	auto p = XMLoadFloat3(&normal);
	auto m = XMLoadFloat4x4(&this->GetGlobalWorldMatrix());
	auto r = XMVector3TransformNormal(p, m);
	XMFLOAT3 result;
	XMStoreFloat3(&result, XMVector3Normalize(r));
	return result;
}

XMFLOAT3 KG::Component::TransformComponent::GetWorldLook() const
{
	return GlobalTransformNormal(Math::look);
}

XMFLOAT3 KG::Component::TransformComponent::GetWorldUp() const
{
	return GlobalTransformNormal(Math::up);
}

XMFLOAT3 KG::Component::TransformComponent::GetWorldRight() const
{
	return GlobalTransformNormal(Math::right);
}

void KG::Component::TransformComponent::OnChangeParent()
{
	//계산 되기 전
	auto prevGlobalMatrix = this->globalWorldMatrix;
	auto newParentGlobalMatrix = this->GetParent()->GetGlobalWorldMatrix();
	auto inverseNpg = KG::Math::Matrix4x4::Inverse(newParentGlobalMatrix);
	auto newLocalMatrix = KG::Math::Matrix4x4::Multiply(inverseNpg, prevGlobalMatrix);
	XMVECTOR s{};
	XMVECTOR r{};
	XMVECTOR t{};
	KG::Math::Matrix4x4::XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&newLocalMatrix));
    this->SetScale(s);
	this->SetRotation(r);
	this->SetPosition(t);

	// pG = npg * x;											
	this->TurnOnLocalDirtyFlag();
}

void KG::Component::TransformComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->rotationQautProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
    this->TurnOnLocalDirtyFlag();
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
