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
	SetupFiltering(filterGroup, maskGroup);
}

KG::Component::DynamicRigidComponent::DynamicRigidComponent()
	:
	KG::Component::IRigidComponent(),
	applyProp("Apply", this->apply)
{
}

void KG::Component::DynamicRigidComponent::PostUpdate(float timeElapsed)
{
	if (apply) {
		// cb.position = Math::Vector3::Add(cb.position, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));
		// PxTransform t = actor->getGlobalPose();
		// t.p = { cb.position.x, cb.position.y, cb.position.z };
		// auto objectQuat = rigid->GetGameObject()->GetTransform()->GetRotation();
		// DirectX::XMStoreFloat4(&objectQuat, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&objectQuat)) * // DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&cb.rotation))));
		// t.q = PxQuat(objectQuat.x, objectQuat.y, objectQuat.z, objectQuat.w);
		// actor->setGlobalPose(t);



		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, false);
		physx::PxVec3 p = actor->getGlobalPose().p;
		physx::PxQuat q = actor->getGlobalPose().q;
		transform->SetPosition(p.x - collisionBox.position.x, p.y - collisionBox.position.y, p.z - collisionBox.position.z);
		transform->SetRotation(q.x, q.y, q.z, q.w);
	}
	else {
		this->actor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);
		this->actor->setLinearVelocity(physx::PxVec3(0, 0, 0));

		auto position = Math::Vector3::Add(collisionBox.position, transform->GetWorldPosition());
		physx::PxTransform t = actor->getGlobalPose();
		t.p = { position.x, position.y, position.z };

		auto objectQuat = transform->GetRotation();		// ???? ???? ???????? ???????? ???? ??????????????
		DirectX::XMStoreFloat4(&objectQuat, DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&objectQuat)) * DirectX::XMMatrixRotationRollPitchYawFromVector(DirectX::XMLoadFloat3(&collisionBox.rotation))));
		t.q = physx::PxQuat(objectQuat.x, objectQuat.y, objectQuat.z, objectQuat.w);
		actor->setGlobalPose(t);


		// DirectX::XMFLOAT4X4 worldMat = gameObject->GetTransform()->GetGlobalWorldMatrix();
		// XMFLOAT3 pos = Math::Vector3::Add(collisionBox.position, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));
		// 
		// physx::PxTransform t = actor->getGlobalPose();
		// t.p = { pos.x,pos.y, pos.z };
		// actor->setGlobalPose(t);
	}
}

void KG::Component::DynamicRigidComponent::Update(float timeElapsed)
{

}

void KG::Component::DynamicRigidComponent::Move(DirectX::XMFLOAT3 direction, float speed) {
    auto vec = actor->getLinearVelocity();
	actor->setLinearVelocity(physx::PxVec3(direction.x * speed, vec.y, direction.z * speed) );
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

void KG::Component::DynamicRigidComponent::AddTorque(DirectX::XMFLOAT3 axis, float power)
{
	// actor->addTorque(physx::PxVec3{ axis.x, axis.y, axis.z } *power);
	actor->setAngularVelocity(physx::PxVec3{ axis.x, axis.y, axis.z } *power);
}

void KG::Component::DynamicRigidComponent::SetRotation(DirectX::XMFLOAT4 quat)
{
	auto pose = actor->getGlobalPose();
	pose.q = physx::PxQuat(quat.x, quat.y, quat.z, quat.w);
	actor->setGlobalPose(pose);
}

void KG::Component::DynamicRigidComponent::SetPosition(DirectX::XMFLOAT3 pos)
{
	auto pose = actor->getGlobalPose();
	pose.p = physx::PxVec3(pos.x, pos.y, pos.z);
	actor->setGlobalPose(pose);
}

DirectX::XMFLOAT3 KG::Component::DynamicRigidComponent::GetVelocity() const
{
	auto vel = this->actor->getLinearVelocity();
	return DirectX::XMFLOAT3(vel.x, vel.y, vel.z);
}

void KG::Component::DynamicRigidComponent::ReleaseActor()
{
	auto* inst = KG::Physics::PhysicsScene::GetInstance();
	inst->ReleaseActor(this);
}

void KG::Component::DynamicRigidComponent::SetLinearLock(bool x, bool y, bool z) {
	// dyn ????->setRigidDynamicLockFlags(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z | PxRigidDynamicLockFlag::eLOCK_ANGULAR_X | PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
	physx::PxRigidDynamicLockFlags p;
	if (x)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
	if (y)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
	if (z)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
	this->actor->setRigidDynamicLockFlags(p);

	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, x);
	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, y);
	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, z);
}

void KG::Component::DynamicRigidComponent::SetAngularLock(bool x, bool y, bool z)
{
	physx::PxRigidDynamicLockFlags p;
	if (x)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
	if (y)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
	if (z)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
	this->actor->setRigidDynamicLockFlags(p);
	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, x);
	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, y);
	// this->actor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, z);
}

void KG::Component::DynamicRigidComponent::SetRigidFlags(bool ax, bool ay, bool az, bool lx, bool ly, bool lz)
{
	physx::PxRigidDynamicLockFlags p;
	if (lx)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
	if (ly)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
	if (lz)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;
	if (ax)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
	if (ay)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
	if (az)
		p |= physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
	this->actor->setRigidDynamicLockFlags(p);
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
		shape->setQueryFilterData(*filterData);
		if (collisionCallback != nullptr)
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	delete[] shapes;
}

void KG::Component::DynamicRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->applyProp.OnDataLoad(componentElement);
	this->positionProp.OnDataLoad(componentElement);
	this->rotationProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->shapeProp.OnDataLoad(componentElement);
	this->filterProp.OnDataLoad(componentElement);
}

void KG::Component::DynamicRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::DynamicRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->rotationProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->applyProp.OnDataSave(componentElement);
	this->shapeProp.OnDataSave(componentElement);
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
			this->rotationProp.OnDrawGUI();
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
			this->shapeProp.OnDrawGUI();
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->rotationProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->shapeProp.OnDrawGUI();
			ImGui::TreePop();
		}

		this->filterProp.OnDrawGUI();			// ?????? ??????????
		this->applyProp.OnDrawGUI();
		
		// ???? ??????

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		switch (show) {
		case COLLISION_SHAPE::NONE:
			break;
		case COLLISION_SHAPE::BOX:
		{
			XMFLOAT4X4 worldMat = this->gameObject->GetTransform()->GetGlobalWorldMatrix();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&collisionBox.rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.position)));
			
			mat = Math::Matrix4x4::Multiply(mat, worldMat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);


			//???? ?????? ?? ????
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
							this->collisionBox.position = XMFLOAT3(t[0], t[1], t[2]);
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
	SetupFiltering(this->filterGroup, this->maskGroup);
}


void KG::Component::StaticRigidComponent::SetupFiltering(uint32_t filterGroup, uint32_t filterMask)
{
	filterGroup |= static_cast<uint32_t>(FilterGroup::eBOX);
	IRigidComponent::SetupFiltering(filterGroup, filterMask);

	const physx::PxU32 numShapes = this->actor->getNbShapes();
	physx::PxShape** shapes = new physx::PxShape * [numShapes];
	this->actor->getShapes(shapes, numShapes);
	for (physx::PxU32 i = 0; i < numShapes; i++)
	{
		physx::PxShape* shape = shapes[i];
		shape->setSimulationFilterData(*filterData);
		shape->setQueryFilterData(*filterData);
		if (collisionCallback != nullptr)
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, true);
	}
	delete[] shapes;
}

KG::Component::StaticRigidComponent::StaticRigidComponent()
{
}

void KG::Component::StaticRigidComponent::PostUpdate(float timeElapsed)
{
	DirectX::XMFLOAT4X4 worldMat = gameObject->GetTransform()->GetGlobalWorldMatrix();
	XMFLOAT3 pos = Math::Vector3::Add(collisionBox.position, DirectX::XMFLOAT3(worldMat._41, worldMat._42, worldMat._43));

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

void KG::Component::StaticRigidComponent::ReleaseActor()
{
	KG::Physics::PhysicsScene::GetInstance()->ReleaseActor(this);
}

void KG::Component::StaticRigidComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
	this->positionProp.OnDataLoad(componentElement);
	this->scaleProp.OnDataLoad(componentElement);
	this->shapeProp.OnDataLoad(componentElement);
}

void KG::Component::StaticRigidComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	auto* componentElement = parentElement->InsertNewChildElement("Component");
	ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::StaticRigidComponent);
	this->positionProp.OnDataSave(componentElement);
	this->scaleProp.OnDataSave(componentElement);
	this->shapeProp.OnDataSave(componentElement);
}

bool KG::Component::StaticRigidComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::StaticRigidComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
		if (this->isUsing()) {
			this->positionProp.OnDrawGUI();
			this->rotationProp.OnDrawGUI();
			ImGui::TextDisabled("Scaling  (%.3f, %.3f, %.3f)", collisionBox.scale.x, collisionBox.scale.y, collisionBox.scale.z);
			this->shapeProp.OnDrawGUI();
		}
		else if (ImGui::TreeNode("Collision Box")) {
			this->positionProp.OnDrawGUI();
			this->rotationProp.OnDrawGUI();
			this->scaleProp.OnDrawGUI();
			this->shapeProp.OnDrawGUI();
			ImGui::TreePop();
		}

		this->filterProp.OnDrawGUI();

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		switch (show) {
		case COLLISION_SHAPE::NONE:
			break;
		case COLLISION_SHAPE::BOX:
		{
			XMFLOAT4X4 worldMat = this->gameObject->GetTransform()->GetGlobalWorldMatrix();
			XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, XMMatrixScalingFromVector(XMLoadFloat3(&collisionBox.scale)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&collisionBox.rotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&collisionBox.position)));
			mat = Math::Matrix4x4::Multiply(mat, worldMat);

			view = Math::Matrix4x4::Transpose(view);
			proj = Math::Matrix4x4::Transpose(proj);

			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);


			//???? ?????? ?? ????
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
							this->collisionBox.position = XMFLOAT3(t[0], t[1], t[2]);
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
	filterData->word0 = filterGroup;									// word0 - ???? ?????? ???? ????
	filterData->word1 = filterMask;										// word1 - ???? ?????? ???? ???? ???? - ???? ?????? ???????? ????
	filterData->word2 = this->id;										// ?????? ???????? ??????
}

KG::Component::IRigidComponent::IRigidComponent() 
	:
	positionProp("Position", this->collisionBox.position),
	scaleProp("Scale", this->collisionBox.scale),
	rotationProp("Rotation", this->collisionBox.rotation),
	shapeProp(
		"Show", this->show,
		{
			{COLLISION_SHAPE::NONE, "None"},
			{COLLISION_SHAPE::BOX, "Box"},
		}, false),
	filterProp(
		"CollisionFilter", this->filter,
		{
			{FilterGroup::eNONE, "None"},
			{FilterGroup::eFLOOR, "Floor"},
			{FilterGroup::eBOX, "Box"},
			{FilterGroup::eBUILDING, "Building"},
			{FilterGroup::eENEMY, "Enemy"},
			{FilterGroup::ePLAYER, "Player"},
			{FilterGroup::eBULLET, "Bullet"}
		}, false)
{
    this->scene = static_cast<KG::Physics::IPhysicsScene*>(KG::Physics::PhysicsScene::GetInstance());
}

void KG::Component::IRigidComponent::SetCollisionBox(CollisionBox& box) {
	this->collisionBox = box;
}

void KG::Component::IRigidComponent::SetCollisionCallback(KG::Component::CollisionCallbackFunc&& collisionCallback) { this->collisionCallback = collisionCallback; }

KG::Component::CollisionCallbackFunc KG::Component::IRigidComponent::GetCollisionCallback() { return collisionCallback; }

void KG::Component::IRigidComponent::SetRaycastCallback(KG::Component::RaycastCallbackFunc& raycastCallback) { this->raycastCallback = raycastCallback; }

KG::Component::RaycastCallbackFunc KG::Component::IRigidComponent::GetRaycastCallback() const { return this->raycastCallback; }

void KG::Component::IRigidComponent::AddFilterGroup(FilterGroup filterGroup, FilterGroup filterMask)
{
	this->filterGroup |= static_cast<uint32_t>(filterGroup);
	this->maskGroup |= static_cast<uint32_t>(filterMask);
}

DirectX::XMFLOAT4X4 KG::Component::CollisionBox::GetMatrix() const
{
    DirectX::XMFLOAT4X4 result;
    auto quat = KG::Math::Quaternion::FromEuler(rotation);
    XMStoreFloat4x4(&result,
        XMMatrixAffineTransformation(XMLoadFloat3(&this->scale), XMVectorZero(), XMLoadFloat4(&quat), XMLoadFloat3(&this->position))
    );
    return result;
}
