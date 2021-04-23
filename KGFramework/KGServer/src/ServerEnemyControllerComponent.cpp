#include "ServerEnemyControllerComponent.h"
#include "PhysicsComponent.h"
#include "Transform.h"
#include "imgui/imgui.h"
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

	angleTo = std::atan2(goal.z, goal.x);
	
	rotateClockwise = IsClockwise();

	// 나중에는 이동 불가능한 위치 선택시 false 리턴
	return true;
}

bool KG::Component::SEnemyControllerComponent::RotateToGoal()
{
	int rotDir = 1;
	bool completeRotate = false;
	if (rotateClockwise) {
		if (!IsClockwise()) {
			// 회전 완료
			completeRotate = true;
			// rigid->GetActor()
		}
		else
			rotDir = 1;
	}
	else {
		if (IsClockwise()) {
			// 회전 완료
			completeRotate = true;
		}
		else
			rotDir = -1;
	}

	if (completeRotate) {
		if (rigid) {
			rigid->SetRotation(gameObject->GetTransform()->GetRotation());
			rigid->SetAngularVelocity(DirectX::XMFLOAT3(0, 0, 0));
		}
	}
	else {
		if (rigid)
			rigid->SetAngularVelocity(DirectX::XMFLOAT3(0, rotDir, 0));
	}

	return completeRotate;
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

	return (flag_x && /*flag_y &&*/ flag_z);
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

bool KG::Component::SEnemyControllerComponent::IsClockwise() const
{
	auto objAngle = gameObject->GetTransform()->GetEulerDegree();
	float curAngle = std::atan2(objAngle.y, objAngle.x);

	return !(abs(angleTo - curAngle) <= 180);
}

void KG::Component::SEnemyControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
	SBaseComponent::OnCreate(obj);
	this->center = this->gameObject->GetTransform()->GetWorldPosition();
	// this->rigid = this->gameObject->GetComponent<KG::Component::IRigidComponent>();
	// this->anim = this->gameObject->GetComponent<AnimationControllerComponent>();
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
		if (RotateToGoal())
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
			ImGui::TextDisabled("Center : (%f, %f, %f)", goal.x, goal.y, goal.z);
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