#include "pch.h"
#include "ClientCharacterComponent.h"
#include "Transform.h"
#include "IAnimationComponent.h"
#include "PhysicsComponent.h"
#include "PhysicsScene.h"


static struct SoldierAnimSet
{
	static constexpr auto standing = "Soldier@Standing.fbx"_id;
	static constexpr auto sprint_f = "Soldier@SprintForward.fbx"_id;
	static constexpr auto sprint_fl = "Soldier@SprintForwardLeft.fbx"_id;
	static constexpr auto sprint_fr = "Soldier@Standing.SprintForwardRight"_id;
	static constexpr auto walk_f = "Soldier@WalkForward.fbx"_id;
	static constexpr auto walk_fl = "Soldier@WalkForwardLeft.fbx"_id;
	static constexpr auto walk_fr = "Soldier@WalkForwardRight.fbx"_id;
	static constexpr auto walk_l = "Soldier@WalkLeft.fbx"_id;
	static constexpr auto walk_r = "Soldier@WalkRight.fbx"_id;
	static constexpr auto walk_b = "Soldier@WalkBackward.fbx"_id;
	static constexpr auto walk_bl = "Soldier@WalkBackwardLeft.fbx"_id;
	static constexpr auto walk_br = "Soldier@WalkBackwardRight.fbx"_id;
};

static struct VectorAnimSet
{
	static constexpr auto idle = "Vector@Idle.FBX"_id;
	static constexpr auto reload = "Vector@Reload.FBX"_id;
	static constexpr auto reload_e = "Vector@ReloadEmpty.FBX"_id;
	static constexpr auto fire = "Vector@Fire.FBX"_id;
	static constexpr auto fireAim = "Vector@Fire Aim.FBX"_id;
};



void KG::Component::CCharacterComponent::ProcessMoveAnim()
{
	if ( this->forwardValue >= this->inputMinimum )
	{
		//¾Õ
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fr, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_f, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else if ( this->forwardValue <= -this->inputMinimum )
	{
		//µÚ
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_br, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_bl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_b, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
	else
	{
		if ( this->rightValue >= this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_r, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else if ( this->rightValue <= -this->inputMinimum )
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_l, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
		else
		{
			this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::standing, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
		}
	}
}

void KG::Component::CCharacterComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->transform = this->GetGameObject()->GetComponent<TransformComponent>();
	this->characterAnimation = this->GetGameObject()->GetComponent<IAnimationControllerComponent>();
	this->rotationTrasnform = this->GetGameObject()->GetChild()->GetTransform();
	this->physics = this->gameObject->GetComponent<DynamicRigidComponent>();
	this->physics->SetApply(false);
}

void KG::Component::CCharacterComponent::Update(float elapsedTime)
{
	this->ProcessMoveAnim();
}

bool KG::Component::CCharacterComponent::OnDrawGUI()
{
	if ( ImGui::ComponentHeader<CCharacterComponent>() )
	{
		ImGui::Text("f-v : %f", this->forwardValue);
		ImGui::Text("r-v : %f", this->rightValue);
	}
	return false;
}

bool KG::Component::CCharacterComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch ( type )
	{
		case KG::Packet::PacketType::SC_PLAYER_DATA:
		{
			auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_DATA>(packet);
			//this->physics->SetPosition(ScenePacket->position);
			this->transform->SetPosition(ScenePacket->position);
			this->rotationTrasnform->SetRotation(ScenePacket->rotation);
			this->rightValue = ScenePacket->rightValue;
			this->forwardValue = ScenePacket->forwardValue;
			return true;
		}
	}
	return false;
}

