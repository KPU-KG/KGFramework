#include "PhysicsComponent.h"
#include "GameObject.h"
#include "PxPhysicsAPI.h"
#include "Transform.h"
#include "PhysicsScene.h"
#include "Scene.h"
#include "MathHelper.h"
#include <unordered_map>
#include "InputManager.h"

void KG::Component::DynamicRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	IRigidComponent::OnCreate(gameObject);
	KG::Physics::PhysicsScene::GetInstance()->AddDynamicActor(this);
	dynamic = true;

	// SetCollisionCallback([this](KG::Component::IRigidComponent* my, KG::Component::IRigidComponent* other) {
	// 	DebugNormalMessage("callback!!")
	// 	});

	SetupFiltering(static_cast<unsigned int>(filter), 0);
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
		}, false),
	filterProp(
		"CollisionFilter", this->filter,
		{
			{FilterGroup::eBOX, "Box"},
			{FilterGroup::eBUILDING, "Building"},
			{FilterGroup::eENEMY, "Enemy"},
			{FilterGroup::eFLOOR, "Floor"},
			{FilterGroup::ePLAYER, "Player"}
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
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		this->actor->setLinearVelocity(physx::PxVec3(0, 0, 0));

		DirectX::XMFLOAT4X4 worldMat = gameObject->GetTransform()->GetGlobalWorldMatrix();
		XMFLOAT3 pos = Math::Vector3::Add(collisionBox.center, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));

		physx::PxTransform t = actor->getGlobalPose();
		t.p = { pos.x,pos.y, pos.z };
		actor->setGlobalPose(t);
	}
}

void KG::Component::DynamicRigidComponent::Update(float timeElapsed)
{
	if (updateLambda != nullptr)
		updateLambda();
}

void KG::Component::DynamicRigidComponent::Move(DirectX::XMFLOAT3 direction, float speed) {
	actor->setLinearVelocity(physx::PxVec3(direction.x, direction.y, direction.z) * speed * 100);
}

void KG::Component::DynamicRigidComponent::SetActor(physx::PxRigidDynamic* actor)
{
	this->actor = actor;
}

void KG::Component::DynamicRigidComponent::AddForce(DirectX::XMFLOAT3 dir, float distance) {
	actor->addForce(physx::PxVec3(dir.x, dir.y, dir.z) * distance * actor->getMass());
}

void KG::Component::DynamicRigidComponent::SetVelocity(DirectX::XMFLOAT3 dir, float distance) {
	actor->setLinearVelocity(physx::PxVec3(dir.x, dir.y, dir.z) * distance);
}


void KG::Component::DynamicRigidComponent::SetupFiltering(unsigned int filterGroup, unsigned int filterMask)
{
	IRigidComponent::SetupFiltering(filterGroup, filterMask);
	const physx::PxU32 numShapes = this->actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape * [numShapes];
	this->actor->getShapes(shapes, numShapes);
	for (physx::PxU32 i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(*filterData);
		if (callback != nullptr)
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	delete[] shapes;
}

void KG::Component::DynamicRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->applyProp.OnDataLoad(componentElement);
	this->showProp.OnDataLoad(componentElement);
	this->filterProp.OnDataLoad(componentElement);
}

void KG::Component::DynamicRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::DynamicRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
	this->showProp.OnDataSave(componentElement);
	this->filterProp.OnDataSave(componentElement);
}

bool KG::Component::DynamicRigidComponent::OnDrawGUI()
{
	if (gameObject == nullptr)
		return false;

	if (ImGui::ComponentHeader<KG::Component::DynamicRigidComponent>()) {
			ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			this->positionProp.OnDrawGUI();
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
			this->showProp.OnDrawGUI();
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->showProp.OnDrawGUI();
			ImGui::TreePop();
		}

		this->filterProp.OnDrawGUI();			// 나중에 고정시킬것
		this->applyProp.OnDrawGUI();
		
		// 큐브 그리기

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		switch (show) {
		case SHOW_COLLISION_BOX::NONE:
			break;
		case SHOW_COLLISION_BOX::GRID:
			// 일단 닫아둔다..
			/*
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
			break;*/
		case SHOW_COLLISION_BOX::BOX:
		{
			XMFLOAT4X4 worldMat = this->gameObject->GetTransform()->GetGlobalWorldMatrix();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)));

			mat = Math::Matrix4x4::Multiply(mat, worldMat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);


			//물리 작업용 툴 섹션
			{
				static ImGuizmo::OPERATION currentGizmoOperation = (ImGuizmo::OPERATION)0;
				static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);
				XMFLOAT4X4 delta;
				float t[3] = { 0,0,0 }; float r[3] = { 0,0,0 }; float s[3] = { 0,0,0 };

				ImGui::Text("Edit Mode");
				if ( ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE) )
					currentGizmoOperation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
				if ( ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE) )
					currentGizmoOperation = ImGuizmo::SCALE;
				ImGui::SameLine();
				if ( ImGui::RadioButton("Off", currentGizmoOperation == 0) )
					currentGizmoOperation = (ImGuizmo::OPERATION)0;

				//if ( ImGui::RadioButton("RotateX", currentGizmoOperation == ImGuizmo::ROTATE_X) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_X;
				//ImGui::SameLine();
				//if ( ImGui::RadioButton("RotateY", currentGizmoOperation == ImGuizmo::ROTATE_Y) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_Y;
				//ImGui::SameLine();
				//if ( ImGui::RadioButton("RotateZ", currentGizmoOperation == ImGuizmo::ROTATE_Z) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_Z;

				if ( currentGizmoOperation != 0 && ImGuizmo::Manipulate((float*)view.m, (float*)proj.m, currentGizmoOperation, currentGizmoMode, (float*)mat.m, NULL, NULL) )
				{
					if ( this->gameObject->GetTransform()->GetParent() )
					{
						auto parentWorld = this->gameObject->GetTransform()->GetParent()->GetGlobalWorldMatrix();
						auto parentWorldMat = XMLoadFloat4x4(&parentWorld);
						auto currentWorldMat = XMLoadFloat4x4(&mat);
						auto currentLocal = XMMatrixMultiply(currentWorldMat, XMMatrixInverse(NULL, parentWorldMat));
						XMStoreFloat4x4(&delta, currentLocal);
					}
					ImGuizmo::DecomposeMatrixToComponents((float*)delta.m, t, r, s);
					//auto euler = this->GetEulerDegree();
					switch ( currentGizmoOperation )
					{
						case ImGuizmo::TRANSLATE:
							this->collisionBox.center = XMFLOAT3(t[0], t[1], t[2]);
							break;
						//case ImGuizmo::ROTATE_X:
						//	this->RotateAxis(this->GetRight(), this->eulerRotation.x - r[0]);
						//	break;
						//case ImGuizmo::ROTATE_Y:
						//	this->RotateAxis(this->GetUp(), r[1] - this->eulerRotation.y);
						//	break;
						//case ImGuizmo::ROTATE_Z:
						//	this->RotateAxis(this->GetLook(), r[2] - this->eulerRotation.z);
							break;
						case ImGuizmo::SCALE:
							this->collisionBox.scale = XMFLOAT3(s[0], s[1], s[2]);
							break;
					}
				}

			}




			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}
			break;
		}

	}
	return false;
}

void KG::Component::StaticRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	IRigidComponent::OnCreate(gameObject);
	KG::Physics::PhysicsScene::GetInstance()->AddStaticActor(this);
	dynamic = false;
	SetupFiltering(static_cast<unsigned int>(filter), 0);
}


void KG::Component::StaticRigidComponent::SetupFiltering(uint32_t filterGroup, uint32_t filterMask)
{
	IRigidComponent::SetupFiltering(filterGroup, filterMask);

	const physx::PxU32 numShapes = this->actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape * [numShapes];
	this->actor->getShapes(shapes, numShapes);
	for (physx::PxU32 i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(*filterData);
		if (callback != nullptr)
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	delete[] shapes;
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
		},false),
	filterProp(
		"CollisionFilter", this->filter,
		{
			{FilterGroup::eBOX, "Box"},
			{FilterGroup::eBUILDING, "Building"},
			{FilterGroup::eENEMY, "Enemy"},
			{FilterGroup::eFLOOR, "Floor"},
			{FilterGroup::ePLAYER, "Player"}
		}, false)
{
}

void KG::Component::StaticRigidComponent::PostUpdate(float timeElapsed)
{
	DirectX::XMFLOAT4X4 worldMat = gameObject->GetTransform()->GetGlobalWorldMatrix();
	XMFLOAT3 pos = Math::Vector3::Add(collisionBox.center, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));

	physx::PxTransform t = actor->getGlobalPose();
	t.p = { pos.x,pos.y, pos.z };
	actor->setGlobalPose(t);
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
			this->showProp.OnDrawGUI();
			ImGui::TreePop();
		}

		this->filterProp.OnDrawGUI();

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		switch (show) {
		case SHOW_COLLISION_BOX::NONE:
			break;
		case SHOW_COLLISION_BOX::GRID:
			// 지금은 다른 작업이 우선
		case SHOW_COLLISION_BOX::BOX:
		{
			XMFLOAT4X4 worldMat = this->gameObject->GetTransform()->GetGlobalWorldMatrix();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.center)));

			mat = Math::Matrix4x4::Multiply(mat, worldMat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);


			//물리 작업용 툴 섹션
			{
				static ImGuizmo::OPERATION currentGizmoOperation = (ImGuizmo::OPERATION)0;
				static ImGuizmo::MODE currentGizmoMode(ImGuizmo::LOCAL);
				XMFLOAT4X4 delta;
				float t[3] = { 0,0,0 }; float r[3] = { 0,0,0 }; float s[3] = { 0,0,0 };

				ImGui::Text("Edit Mode");
				if ( ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE) )
					currentGizmoOperation = ImGuizmo::TRANSLATE;
				ImGui::SameLine();
				if ( ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE) )
					currentGizmoOperation = ImGuizmo::SCALE;
				ImGui::SameLine();
				if ( ImGui::RadioButton("Off", currentGizmoOperation == 0) )
					currentGizmoOperation = (ImGuizmo::OPERATION)0;

				//if ( ImGui::RadioButton("RotateX", currentGizmoOperation == ImGuizmo::ROTATE_X) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_X;
				//ImGui::SameLine();
				//if ( ImGui::RadioButton("RotateY", currentGizmoOperation == ImGuizmo::ROTATE_Y) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_Y;
				//ImGui::SameLine();
				//if ( ImGui::RadioButton("RotateZ", currentGizmoOperation == ImGuizmo::ROTATE_Z) )
				//	currentGizmoOperation = ImGuizmo::ROTATE_Z;

				if ( currentGizmoOperation != 0 && ImGuizmo::Manipulate((float*)view.m, (float*)proj.m, currentGizmoOperation, currentGizmoMode, (float*)mat.m, NULL, NULL) )
				{
					if ( this->gameObject->GetTransform()->GetParent() )
					{
						auto parentWorld = this->gameObject->GetTransform()->GetParent()->GetGlobalWorldMatrix();
						auto parentWorldMat = XMLoadFloat4x4(&parentWorld);
						auto currentWorldMat = XMLoadFloat4x4(&mat);
						auto currentLocal = XMMatrixMultiply(currentWorldMat, XMMatrixInverse(NULL, parentWorldMat));
						XMStoreFloat4x4(&delta, currentLocal);
					}
					ImGuizmo::DecomposeMatrixToComponents((float*)delta.m, t, r, s);
					//auto euler = this->GetEulerDegree();
					switch ( currentGizmoOperation )
					{
						case ImGuizmo::TRANSLATE:
							this->collisionBox.center = XMFLOAT3(t[0], t[1], t[2]);
							break;
							//case ImGuizmo::ROTATE_X:
							//	this->RotateAxis(this->GetRight(), this->eulerRotation.x - r[0]);
							//	break;
							//case ImGuizmo::ROTATE_Y:
							//	this->RotateAxis(this->GetUp(), r[1] - this->eulerRotation.y);
							//	break;
							//case ImGuizmo::ROTATE_Z:
							//	this->RotateAxis(this->GetLook(), r[2] - this->eulerRotation.z);
							break;
						case ImGuizmo::SCALE:
							this->collisionBox.scale = XMFLOAT3(s[0], s[1], s[2]);
							break;
					}
				}

			}




			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
		}
		break;
		}
	}
	return false;
}

void KG::Component::IRigidComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	KG::Component::IComponent::OnCreate(gameObject);
	transform = gameObject->GetComponent<KG::Component::TransformComponent>();
}

void KG::Component::IRigidComponent::SetupFiltering(uint32_t filterGroup, uint32_t filterMask) {
	filterData = new physx::PxFilterData;
	filterData->word0 = filterGroup;									// word0 - 해당 액터의 충돌 필터
	filterData->word1 = filterMask;										// word1 - 충돌 처리를 하지 않을 필터 - 이건 나중에 테이블로 정리
	filterData->word2 = this->id;										// 피직스 씬에서의 아이디

}
