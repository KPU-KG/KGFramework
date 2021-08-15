#include "pch.h"
#include "ClientCharacterComponent.h"
#include "Transform.h"
#include "IAnimationComponent.h"
#include "PhysicsComponent.h"
#include "PhysicsScene.h"
#include "ISoundComponent.h"
#include "IParticleEmitterComponent.h"
#include "IRender3DComponent.h"
#include "IRender2DComponent.h"
#include "Scene.h"

using namespace KG::Math::Literal;

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
		//앞
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
		//뒤
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


void KG::Component::CCharacterComponent::ProcessMove(float elapsedTime)
{
	bool forwardInput = false;
	bool rightInput = false;
	float speed = this->inputs.stateShift ? 10.0f : 6.0f;
	speed *= speedValue;
	if (inputs.stateW)
	{
		this->forwardValue += inputRatio * +1 * elapsedTime;
		forwardInput = true;
	}
	if (inputs.stateS)
	{
		this->forwardValue += inputRatio * -1 * elapsedTime;
		forwardInput = true;
	}
	if (inputs.stateD)
	{
		this->rightValue += inputRatio * +1 * elapsedTime;
		rightInput = true;
	}
	if (inputs.stateA)
	{
		this->rightValue += inputRatio * -1 * elapsedTime;
		rightInput = true;
	}

	if (!forwardInput)
	{
		if (abs(forwardValue) > this->inputMinimum)
		{
			forwardValue += inputRetRatio * ((forwardValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			forwardValue = 0.0f;
		}
	}

	if (!rightInput)
	{
		if (abs(rightValue) > this->inputMinimum)
		{
			rightValue += inputRetRatio * ((rightValue > 0) ? -1 : 1) * elapsedTime;
		}
		else
		{
			rightValue = 0.0f;
		}
	}
	forwardValue = KG::Math::Clamp(forwardValue, -1.0f, 1.0f);
	rightValue = KG::Math::Clamp(rightValue, -1.0f, 1.0f);

	if (abs(this->forwardValue) >= this->inputMinimum)
	{
		this->transform->Translate(this->rotationTransform->GetLook() * speed * elapsedTime * this->forwardValue);
	}
	if (abs(this->rightValue) >= this->inputMinimum)
	{
		this->transform->Translate(this->rotationTransform->GetRight() * speed * elapsedTime * this->rightValue);
	}
}

void KG::Component::CCharacterComponent::OnCreate(KG::Core::GameObject* obj)
{
	this->transform = this->GetGameObject()->GetComponent<TransformComponent>();
	this->characterAnimation = this->GetGameObject()->GetComponent<IAnimationControllerComponent>();
	this->rotationTransform = this->GetGameObject()->GetChild()->GetTransform();
	this->physics = this->gameObject->GetComponent<DynamicRigidComponent>();
	this->physics->SetApply(false);
	this->sound = this->gameObject->GetComponent<ISoundComponent>();
    this->gameObject->GetScene()->GetComponentProvider()->AddComponentToObject(KG::Component::ComponentID<IParticleEmitterComponent>::id(), this->gameObject);
    this->particle = this->gameObject->GetComponent<IParticleEmitterComponent>();
    this->render3d = this->gameObject->FindChildObject("Soldier"_id)->GetComponent<IRender3DComponent>();
    this->hpBar = this->gameObject->FindChildObject("HPBAR"_id)->GetComponent<KG::Component::IRenderSpriteComponent>();
    this->vector3d = this->gameObject->FindChildObject("VectorDrop"_id)->GetComponent<IRender3DComponent>();
}

void KG::Component::CCharacterComponent::Update(float elapsedTime)
{
	this->ProcessMove(elapsedTime);
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

void KG::Component::CCharacterComponent::InterpolatePosition(DirectX::XMFLOAT3 position) {
	//this->physics->SetPosition(position);
	DirectX::XMFLOAT3 t;
	auto a = this->transform->GetWorldPosition();
	if (abs(a.x - position.x) + abs(a.z - position.z) > 3.0f) {
		this->transform->SetPosition(position);
	}
	else if (abs(a.x - position.x) + abs(a.z - position.z) > 1.0f) {
		t.x = (a.x + position.x) / 2;
		t.y = position.y;
		t.z = (a.z + position.z) / 2;
		this->transform->SetPosition(t);
	}
}

bool KG::Component::CCharacterComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
	switch ( type )
	{
		case KG::Packet::PacketType::SC_PLAYER_DATA:
		{
			auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_DATA>(packet);
			//this->transform->SetPosition(ScenePacket->position);
			this->InterpolatePosition(ScenePacket->position);
			this->rotationTransform->SetRotation(ScenePacket->rotation);
			this->rightValue = ScenePacket->rightValue;
			this->forwardValue = ScenePacket->forwardValue;
			this->inputs = ScenePacket->inputs; // 인풋 갱신, 클라 업데이트 -> 무브 프로세스
            this->hpBar->progress.value = ScenePacket->playerHp;
            if (ScenePacket->playerHp <= 0.1f)
            {
                this->render3d->SetVisible(false);
                this->vector3d->SetVisible(false);
            }
            else
            {
                this->render3d->SetVisible(true);
                this->vector3d->SetVisible(true);
            }
			return true;
		}

		case KG::Packet::PacketType::SC_REMOVE_PLAYER:
		{
			if (this->physics)
				this->physics->ReleaseActor();
			this->gameObject->Destroy();
			return true;
		}

		case KG::Packet::PacketType::SC_FIRE:
		{
            auto* firePacket = KG::Packet::PacketCast<KG::Packet::SC_FIRE>(packet);
            float speed = 50.0f;
            auto start = firePacket->origin;
            auto direction = firePacket->direction * speed;

			if (this->sound)
			{
				int randSound = KG::Math::RandomInt(VECTOR_SOUND::FIRE_1_3D, VECTOR_SOUND::FIRE_4_3D);
				//this->sound->PlayEffectiveSound(randSound);
                this->sound->Play3DSound(randSound, start);
			}

            this->particle->EmitParticle(KG::Utill::HashString("Muzzle"_id), start);
            this->particle->EmitParticle(KG::Utill::HashString("TeamBulletLine"_id), start, direction, 10.0f);
			return true;
		}

	}
	return false;
}

