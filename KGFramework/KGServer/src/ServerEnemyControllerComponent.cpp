#include "ServerEnemyControllerComponent.h"
#include "PhysicsComponent.h"
#include "AnimationComponent.h"
#include "Transform.h"
#include "imgui/imgui.h"
#include "MathHelper.h"
#include <string>
#include <random>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<float> goalRange(-1, 1);

void KG::Component::SEnemyControllerComponent::UpdateState()
{

}

bool KG::Component::SEnemyControllerComponent::SetGoal()
{
	// 나중에는 노드 생성해서 그걸로 변경
	goal.x = goalRange(gen) * range;
	// goal.y = goalRange(gen) * range;
	goal.z = goalRange(gen) * range;

	direction = Math::Vector3::Subtract(goal, transform->GetPosition());
	direction.y = 0;
	XMStoreFloat3(&direction, XMVector3Normalize(XMLoadFloat3(&direction)));

	auto dir = DirectX::XMFLOAT2{ direction.x, direction.z };

	// auto angleTo = std::atan2(goal.z, goal.x);
	// XMStoreFloat4(&quatTo, Math::XMQuaternionRotationXYZ(0, angleTo, 0));
	
	auto look = DirectX::XMFLOAT2{ transform->GetLook().x, transform->GetLook().z };
	// look.y = 0;
	// angleTo = std::atan2(look.z, look.x);

	// XMStoreFloat4(&quatFrom, Math::XMQuaternionRotationXYZ(0, angleTo, 0));
	rotateTimer = 0;
	// 나중에는 이동 불가능한 위치 선택시 false 리턴

	XMStoreFloat2(&angle, DirectX::XMVector2AngleBetweenVectors(XMLoadFloat2(&look), XMLoadFloat2(&dir)));
	return true;
}

bool KG::Component::SEnemyControllerComponent::RotateToGoal(float elapsedTime)
{
	rotateTimer += elapsedTime;
	if (rotateInterval <= rotateTimer) {
		// rigid->SetRotation(quatTo);
		return true;
	}
	else {
		// DirectX::XMFLOAT4 quat;
		DirectX::XMFLOAT4 rot;
		XMStoreFloat4(&rot, XMQuaternionRotationRollPitchYaw(0, angle.x * elapsedTime / rotateInterval, 0));
		gameObject->GetTransform()->Rotate(rot);
		// XMStoreFloat4(&quat, XMQuaternionSlerp(XMLoadFloat4(&quatFrom), XMLoadFloat4(&quatTo), rotateTimer / rotateInterval));

		
		// XMStoreFloat4(&quat, XMQuaternionSlerp(XMLoadFloat4(&quatFrom), XMLoadFloat4(&quatTo), rotateTimer / rotateInterval));
		rigid->SetRotation(transform->GetRotation());
		// rigid->Rotate(DirectX::XMFLOAT3(0, angle.x, 0));
	}
	return false;
}

bool KG::Component::SEnemyControllerComponent::MoveToGoal()
{
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
	idleTimer += elapsedTime;
	if (idleInterval <= idleTimer)
		return true;
	return false;
}

KG::Component::SEnemyControllerComponent::SEnemyControllerComponent()
{

}

void KG::Component::SEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	this->transform = this->gameObject->GetTransform();
	this->center = this->transform->GetWorldPosition();
	this->rigid = this->gameObject->GetComponent<KG::Component::DynamicRigidComponent>();
	this->anim = this->gameObject->GetComponent<AnimationControllerComponent>();
}

void KG::Component::SEnemyControllerComponent::Update(float elapsedTime)
{
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
			auto angle = transform->GetEulerDegree();
			ImGui::TextDisabled("rotation : (%f, %f, %f)", angle.x, angle.y, angle.z);
		}
		else {
			std::string cs("center");
			KG::Utill::ImguiProperty::DrawGUIProperty<DirectX::XMFLOAT3>(cs, this->center);

			std::string rs("range");
			KG::Utill::ImguiProperty::DrawGUIProperty<float>(rs, this->range);
		}
	}
	return false;
}


bool KG::Component::SEnemyControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender)
{
	return false;
}

void KG::Component::SEnemyControllerComponentSystem::OnPostUpdate(float elapsedTime)
{

}

void KG::Component::SEnemyControllerComponentSystem::OnPreRender()
{

}