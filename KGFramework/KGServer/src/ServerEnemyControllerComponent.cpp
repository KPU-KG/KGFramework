#include "pch.h"
#include "ServerEnemyControllerComponent.h"
#include "KGServer.h"
#include "Transform.h"

#include "PhysicsComponent.h"
#include "IAnimationComponent.h"

#include "imgui/imgui.h"
#include "MathHelper.h"

#include <string>
#include <random>
#include "Scene.h"

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> goalRange(-0.5, 0.5);

void KG::Component::SEnemyControllerComponent::UpdateState()
{

}

bool KG::Component::SEnemyControllerComponent::SetGoal()
{
	if (this->nodeCount > 0) {
		if (randomCircuit) {
			std::uniform_int_distribution<int> randomIndex(1, this->nodeCount);
			currentNode = randomIndex(gen) - 1;
		}
		else {
			currentNode = ++currentNode% this->nodeCount;
		}
		goal = this->node[currentNode];
	}
	else {
		goal.x = goalRange(gen) * range + center.x;
		// goal.y = goalRange(gen) * range;
		goal.z = goalRange(gen) * range + center.z;
	}

	direction = Math::Vector3::Subtract(goal, transform->GetPosition());
	direction.y = 0;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

	auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };
	
	auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };
	rotateTimer = 0;
	// 나중에는 이동 불가능한 위치 선택시 false 리턴

	XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	XMFLOAT2 crs;
	XMStoreFloat2(&crs, XMVector2Cross(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	if (crs.x >= 0)
		angle.x *= -1;
	return true;
}

bool KG::Component::SEnemyControllerComponent::RotateToGoal(float elapsedTime)
{
	if (anim) {
		ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	// anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING);
	rotateTimer += elapsedTime;
	if (rotateInterval <= rotateTimer) {
		return true;
	}
	else {
		DirectX::XMFLOAT4 rot;
		float r = angle.x * elapsedTime / rotateInterval;
		angle.y -= abs(r);
		XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, r, 0));
		gameObject->GetTransform()->Rotate(rot);
		rigid->SetRotation(transform->GetRotation());
	}
	return false;
}

bool KG::Component::SEnemyControllerComponent::MoveToGoal()
{
	if (anim) {
		ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	// anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING);

	if (rigid) {
		rigid->SetVelocity(direction, speed);
	}
	// 진행 방향
	bool flag_x = false;
	// bool flag_y = false;
	bool flag_z = false;
	auto curPos = gameObject->GetTransform()->GetPosition();
	if (direction.x <= 0) {
		if (curPos.x <= goal.x)
			flag_x = true;
	}
	else {
		if (curPos.x >= goal.x)
			flag_x = true;
	}
	if (direction.z <= 0) {
		if (curPos.z <= goal.z)
			flag_z = true;
	}
	else {
		if (curPos.z >= goal.z)
			flag_z = true;
	}

	if ((flag_x && /*flag_y &&*/ flag_z)) {
		rigid->SetVelocity(direction, 0);
		return true;
	}

	return false;
}

bool KG::Component::SEnemyControllerComponent::Idle(float elapsedTime)
{
	if (anim) {
		ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::shotSmallCanon, ANIMSTATE_PLAYING, 0.1f, -1);
	}
	//anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::shotSmallCanon, ANIMSTATE_PLAYING);
	idleTimer += elapsedTime;
	if (idleInterval <= idleTimer)
		return true;
	return false;
}

KG::Component::SEnemyControllerComponent::SEnemyControllerComponent()
{

}

void KG::Component::SEnemyControllerComponent::SetCenter(DirectX::XMFLOAT3 center) {
	this->center = center;
}

void KG::Component::SEnemyControllerComponent::SetSpeed(float speed) {
	this->speed = speed;
}

void KG::Component::SEnemyControllerComponent::SetIdleInterval(float interval) {
	this->idleInterval = interval;
}

void KG::Component::SEnemyControllerComponent::SetRotateInterval(float interval) {
	this->rotateInterval = interval;
}

void KG::Component::SEnemyControllerComponent::SetWanderRange(float range) {
	this->range = range;
}

void KG::Component::SEnemyControllerComponent::SetPosition(DirectX::XMFLOAT3 position)
{
	this->rigid->SetPosition(position);
}

void KG::Component::SEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	// this->SetNetObjectId(this->server->GetNewObjectId());

	this->transform = this->gameObject->GetTransform();
	this->center = this->transform->GetWorldPosition();
	this->rigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();
	if (this->rigid)
		this->rigid->SetRaycastCallback(raycastCallback);
	this->anim = this->gameObject->GetComponent<IAnimationControllerComponent>();
}

void KG::Component::SEnemyControllerComponent::Update(float elapsedTime)
{
	if (hp <= 0) {
		if (!isDead && anim->GetCurrentPlayingAnimationIndex() != KG::Component::MechAnimIndex::dead) {
			ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::dead, ANIMSTATE_STOP, 0.1, 1);
			isDead = true;
		}

	}
	else {
		switch (this->action) {
		case EnemyAction::eIDLE:
			if (Idle(elapsedTime))
				action = EnemyAction::eSETGOAL;
			break;
		case EnemyAction::eSETGOAL:
			SetGoal();
			action = EnemyAction::eROTATE;
			break;
		case EnemyAction::eROTATE:
			if (RotateToGoal(elapsedTime))
				action = EnemyAction::eMOVE;
			break;
		case EnemyAction::eMOVE:
			if (MoveToGoal()) {
				action = EnemyAction::eIDLE;
				idleTimer = 0;
			}
			break;
		case EnemyAction::eATTACK:
			break;
		case EnemyAction::eATTACKED:
			break;
		}
	}

	if (this->server) {
		KG::Packet::SC_MOVE_OBJECT p = {};
		p.position = this->transform->GetPosition();
		p.rotation = this->transform->GetRotation();
		this->BroadcastPacket(&p);
	}
}

bool KG::Component::SEnemyControllerComponent::OnDrawGUI()
{
	if (ImGui::ComponentHeader<KG::Component::SEnemyControllerComponent>()) {
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());


		if (this->isUsing()) {
			ImGui::TextDisabled("Center : (%f, %f, %f)", center.x, center.y, center.z);
			ImGui::TextDisabled("Range : %f", range);
			const char* curAction = "None";
			switch (this->action) {
			case EnemyAction::eIDLE:
				curAction = "Idle";
				break;
			case EnemyAction::eSETGOAL:
				curAction = "SetGoal";
				break;
			case EnemyAction::eROTATE:
				curAction = "Rotate";
				break;
			case EnemyAction::eMOVE:
				curAction = "Move";
				break;
			case EnemyAction::eATTACK:
				curAction = "Attack";
				break;
			case EnemyAction::eATTACKED:
				curAction = "Attacked";
				break;
			}
			ImGui::TextDisabled("Action : %s", curAction);
			ImGui::TextDisabled("Goal : (%f, %f, %f)", goal.x, goal.y, goal.z);
			ImGui::TextDisabled("Direction : (%f, %f, %f)", direction.x, direction.y, direction.z);
			auto angle = transform->GetEulerDegree();
			ImGui::TextDisabled("rotation : (%f, %f, %f)", angle.x, angle.y, angle.z);
			ImGui::TextDisabled("this->angle : (%f, %f)", this->angle.x, this->angle.y);
			ImGui::TextDisabled("Cur Node : (%d)", this->currentNode);
		}
		else {
			std::string cs("position");
			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(cs, this->center);

			std::string rs("range");
			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->range);
		}
		std::string cs("random circuit");
		KG::Utill::ImguiProperty::DrawGUIProperty<bool>(cs, this->randomCircuit);

		ImGui::TextDisabled("Circuit Node [%d]", this->nodeCount);

		auto view = this->gameObject->GetScene()->GetMainCameraView();
		auto proj = this->gameObject->GetScene()->GetMainCameraProj();

		view = Math::Matrix4x4::Transpose(view);
		proj = Math::Matrix4x4::Transpose(proj);

		XMFLOAT4X4 mat;

		for (int i = 0; i < nodeCount; ++i) {
			nodeProp[i].OnDrawGUI();
			DirectX::XMStoreFloat4x4(&mat, XMMatrixTranslation(this->node[i].x, this->node[i].y, this->node[i].z));



			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

			ImGuizmo::DrawCubes(
				reinterpret_cast<const float*>(view.m),
				reinterpret_cast<const float*>(proj.m),
				reinterpret_cast<const float*>(mat.m),
				1);
			std::string bn("Delete Node " + std::to_string(i));
			if (ImGui::Button(bn.c_str()))
			{
				if (i != MAX_NODE - 1) {
					memcpy(&this->node[i], &this->node[i + 1], sizeof(DirectX::XMFLOAT3) * (MAX_NODE - i - 1));
				}
				if (i < currentNode) {
					currentNode--;
				}
				nodeCount--;
				this->action = EnemyAction::eSETGOAL;
			}
		}

		if (nodeCount < MAX_NODE) {
			if (ImGui::Button("Add Node"))
			{
				node[nodeCount++] = { 0,0,0 };
			}
		}

	}
	return false;
}

void KG::Component::SEnemyControllerComponent::OnDataLoad(tinyxml2::XMLElement* objectElement)
{
	// for (int i = 0; i < MAX_NODE; ++i)
	// 	nodeProp[i].OnDataLoad(objectElement);
}

void KG::Component::SEnemyControllerComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
	// auto* componentElement = parentElement->InsertNewChildElement("Component");
	// ADD_COMPONENT_ID_TO_ELEMENT(componentElement, KG::Component::SEnemyControllerComponent);
	// for (int i = 0; i < MAX_NODE; ++i)
	// 	nodeProp[i].OnDataSave(parentElement);
}

void KG::Component::SEnemyControllerComponent::SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback) {
	this->raycastCallback = callback;
}

void KG::Component::SEnemyControllerComponent::HitBullet() {
	this->hp -= 1;
}

bool KG::Component::SEnemyControllerComponent::IsDead() const
{
	return isDead;
}

inline void KG::Component::SEnemyControllerComponent::ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime, int repeat) {
	anim->ChangeAnimation(animId, animIndex, nextState, blendingTime, repeat);
	KG::Packet::SC_CHANGE_ANIMATION pa = {};
	pa.animId = animId;
	pa.animIndex = animIndex;
	pa.blendingTime = blendingTime;
	pa.nextState = nextState;
	pa.repeat = repeat;
	this->BroadcastPacket(&pa);
}


bool KG::Component::SEnemyControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

