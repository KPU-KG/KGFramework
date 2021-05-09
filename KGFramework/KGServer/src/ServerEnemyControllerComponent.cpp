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
	if (anim)
		anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walkInPlace, ANIMSTATE_PLAYING);
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
	if (anim)
		anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::walk, ANIMSTATE_PLAYING);

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
	if (anim)
		anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::shotSmallCanon, ANIMSTATE_PLAYING);
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
		anim->ChangeAnimation(KG::Utill::HashString("mech.fbx"_id), KG::Component::MechAnimIndex::dead, ANIMSTATE_PLAYING, 0.1);

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

	KG::Packet::SC_MOVE_OBJECT p = {};
	p.position = this->transform->GetPosition();
	p.rotation = this->transform->GetRotation();
	this->BroadcastPacket(&p);

	KG::Packet::SC_SYNC_ANIMATION pa = {};
	pa.animId = this->anim->GetCurrentPlayingAnimationId();
	pa.animIndex = this->anim->GetCurrentPlayingAnimationIndex();
	pa.timer = this->anim->GetCurrentPlayingAnimationTime();
	this->BroadcastPacket(&pa);


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
			ImGui::TextDisabled("this->angle : (%f, %f)", this->angle.x, this->angle.y);
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
	/*
	switch (type)
	{
	case KG::Packet::PacketType::None:
	case KG::Packet::PacketType::PacketHeader:
	case KG::Packet::PacketType::SC_LOGIN_OK:
	case KG::Packet::PacketType::SC_PLAYER_INIT:
	case KG::Packet::PacketType::SC_ADD_OBJECT:

	case KG::Packet::PacketType::SC_REMOVE_OBJECT:
	case KG::Packet::PacketType::SC_FIRE:
	case KG::Packet::PacketType::SC_ADD_PLAYER:
	case KG::Packet::PacketType::SC_PLAYER_SYNC:
		std::cout << "Error Packet Received\n";
		return false;
	case KG::Packet::PacketType::CS_REQ_LOGIN:
	{
		// auto id = this->server->GetNewObjectId();
		// 
		// //플레이어 추가!
		// this->server->LockWorld();
		// this->server->isConnect = true;
		// auto* playerComp = static_cast<KG::Component::SBaseComponent*>(this->gameObject->GetScene()->CallNetworkCreator("TeamCharacter"_id));
		// playerComp->SetNetObjectId(id);
		// auto* trans = playerComp->GetGameObject()->GetTransform();
		// trans->SetPosition(id, 0, id);
		// this->GetGameObject()->GetTransform()->AddChild(trans);
		// this->server->UnlockWorld();
		// //this->server->Addplayer(trans)
		// 
		// KG::Packet::SC_PLAYER_INIT initPacket = {};
		// initPacket.playerObjectId = id;
		// initPacket.position = KG::Packet::RawFloat3(id, 0, id);
		// initPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		// this->SendPacket(sender, &initPacket);
		// 
		// KG::Packet::SC_ADD_PLAYER addPacket = {};
		// addPacket.playerObjectId = id;
		// addPacket.position = KG::Packet::RawFloat3(id, 0, id);
		// addPacket.rotation = KG::Packet::RawFloat4(0, 0, 0, 1);
		// this->BroadcastPacket(&addPacket, sender);
	}
	return true;
	case KG::Packet::PacketType::CS_INPUT: {
		// this->server->inputs
		// 해당 인풋을 보낸 클라이언트의 인풋 정보 변경
	}
										 return true;
	case KG::Packet::PacketType::CS_FIRE:
		std::cout << "Error Packet Received\n";
		return false;
	}*/
	return false;
}

