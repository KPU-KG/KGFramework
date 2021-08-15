#include "pch.h"
#include "ClientPlayerControllerComponent.h"
#include "Transform.h"
#include "ICameraComponent.h"
#include "IAnimationComponent.h"
#include "IRender2DComponent.h"
#include "IParticleEmitterComponent.h"
#include "IPhysicsScene.h"
#include "PhysicsComponent.h"
#include "ISoundComponent.h"
#include "IRender3DComponent.h"
#include "IPostProcessComponent.h"
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


void KG::Component::CPlayerControllerComponent::OnCreate(KG::Core::GameObject* obj)
{
    IComponent::OnCreate(obj);
    this->postProcess = this->gameObject->GetScene()->GetRootNode()->GetComponent<IPostProcessManagerComponent>();

    auto* spine = this->gameObject->FindChildObject("Spine3"_id);
    spine->GetTransform()->SetScale(0, 0, 0);

    this->characterTransform = this->gameObject->GetComponent<TransformComponent>();
    this->characterAnimation = this->gameObject->GetComponent<IAnimationControllerComponent>();
    this->physics = this->gameObject->GetComponent<DynamicRigidComponent>();
    this->sound = this->gameObject->GetComponent<ISoundComponent>();

    auto* cameraObject = this->gameObject->FindChildObject("FPCamera"_id);
    this->cameraTransform = cameraObject->GetTransform();
    this->camera = cameraObject->GetComponent<ICameraComponent>();

    auto* vectorObject = this->gameObject->FindChildObject("Vector"_id);
    this->vectorRender = vectorObject->FindChildObject("Mesh_Vector"_id)->GetComponent<IRender3DComponent>();
    this->armRender = vectorObject->FindChildObject("Mesh_Arm"_id)->GetComponent<IRender3DComponent>();
    this->aimRender = vectorObject->FindChildObject("Cube.006"_id)->GetComponent<IRender3DComponent>();
    this->vectorAnimation = vectorObject->GetComponent<IAnimationControllerComponent>();

    auto* particleObject = this->gameObject->FindChildObject("ParticleGenerator"_id);
    this->particleGen = particleObject->GetComponent<IParticleEmitterComponent>();

    auto* digit_0_Obj = this->gameObject->FindChildObject("DIGIT_0"_id);
    auto* digit_1_Obj = this->gameObject->FindChildObject("DIGIT_1"_id);
    auto* hpBar_Obj = this->gameObject->FindChildObject("HPUI"_id);
    digit_0 = digit_0_Obj->GetComponent<IRender2DComponent>();
    digit_1 = digit_1_Obj->GetComponent<IRender2DComponent>();
    hpbar = hpBar_Obj->GetComponent<IRender2DComponent>();
    this->OnChangeBulletCount(this->bulletCount);

    KG::Component::ParticleDesc muzzleDesc;
    muzzleDesc.baselifeTime = 0.055f;
    muzzleDesc.baseEmitCount = 1;
    muzzleDesc.baseSize.x = 0.35;
    muzzleDesc.baseSize.y = 0.35;
    muzzleDesc.type = ParticleType::Add;
    muzzleDesc.color.SetByFloat(1, 0.65, 0);
    muzzleDesc.materialId = KG::Utill::HashString("Muzzle_06");

    this->particleGen->AddParticleDesc(KG::Utill::HashString("Muzzle"), muzzleDesc);

    muzzleDesc.baseSize.x = 8.0;
    muzzleDesc.baseSize.y = 8.0;
    this->particleGen->AddParticleDesc(KG::Utill::HashString("EnemyHit"), muzzleDesc);


    KG::Component::ParticleDesc sparkDesc;
    sparkDesc.baselifeTime = 1.0f;
    sparkDesc.baseEmitCount = 20;
    sparkDesc.rangeEmitCount = 20;
    sparkDesc.baseSize.x = 0.075;
    sparkDesc.baseSize.y = 0.075;
    sparkDesc.rangeSpeed.x = 5;
    sparkDesc.rangeSpeed.y = 5;
    sparkDesc.rangeSpeed.z = 5;
    sparkDesc.type = ParticleType::Add;
    sparkDesc.color.SetByFloat(1, 1, 0);
    sparkDesc.materialId = KG::Utill::HashString("SparkParticle");

    this->particleGen->AddParticleDesc(KG::Utill::HashString("Spark"), sparkDesc);

    KG::Component::ParticleDesc bulletLine;
    bulletLine.baselifeTime = 3.0f;
    bulletLine.baseEmitCount = 1;
    bulletLine.rangeEmitCount = 0;
    bulletLine.baseSize.x = 0.25;
    bulletLine.baseSize.y = 0.25;
    bulletLine.type = ParticleType::Add;
    bulletLine.color.SetByFloat(1, 1, 0);
    bulletLine.materialId = KG::Utill::HashString("SparkParticle");

    this->particleGen->AddParticleDesc(KG::Utill::HashString("BulletLine"), bulletLine);

    bulletLine.baseSize.x = 0.5;
    bulletLine.baseSize.y = 0.5;

    this->particleGen->AddParticleDesc(KG::Utill::HashString("TeamBulletLine"), bulletLine);

}

void KG::Component::CPlayerControllerComponent::Update(float elapsedTime)
{
    this->InternalUpdate(elapsedTime);
    this->SendUpdate(elapsedTime);
}

bool KG::Component::CPlayerControllerComponent::OnDrawGUI()
{
    return false;
}

bool KG::Component::CPlayerControllerComponent::OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type)
{
    switch ( type )
    {
        case KG::Packet::PacketType::SC_PLAYER_DATA:
        {
            auto* ScenePacket = KG::Packet::PacketCast<KG::Packet::SC_PLAYER_DATA>(packet);
            this->characterTransform->SetPosition(ScenePacket->position);
            hpbar->progress.value = ScenePacket->playerHp;
            if (ScenePacket->playerHp <= 0 && this->isActive) {
                this->isActive = false;
                postProcess->SetActive(5, true);
                this->armRender->SetVisible(this->isActive);
                this->aimRender->SetVisible(this->isActive);
                this->vectorRender->SetVisible(this->isActive);
            }
            else if (ScenePacket->playerHp > 0 && !this->isActive) {
                this->isActive = true;
                postProcess->SetActive(5, false);
                this->armRender->SetVisible(this->isActive);
                this->aimRender->SetVisible(this->isActive);
                this->vectorRender->SetVisible(this->isActive);
            }
            //회전 정보 무시 = 에임 랙걸림
            return true;
        }
    }
    return false;
}

void KG::Component::CPlayerControllerComponent::InternalUpdate(float elapsedTime)
{
    if (this->isActive) {
        auto* input = KG::Input::InputManager::GetInputManager();
        if (input->IsTouching('9'))
        {
            input->SetMouseCapture(false);
        }

        if (input->IsTouching('0'))
        {
            input->SetMouseCapture(true);
        }
        this->ProcessMove(elapsedTime);
        this->ProcessMoveAnimation(elapsedTime);
        this->ProcessMouse(elapsedTime);
        if (!this->CheckReloading())
        {
            if (input->GetKeyState('R') == Input::KeyState::Down)
            {
                this->TryReload(elapsedTime);
            }
            else
            {
                this->ProcessShoot(elapsedTime);
            }
        }
    }
}

void KG::Component::CPlayerControllerComponent::SendUpdate(float elapsedTime)
{
    this->sendPacketTimer += elapsedTime;
    KG::Packet::CS_INPUT inputPacket = {};
    auto input = KG::Input::InputManager::GetInputManager();
    inputPacket.stateW = static_cast<unsigned char>(input->GetKeyState('W'));
    inputPacket.stateA = static_cast<unsigned char>(input->GetKeyState('A'));
    inputPacket.stateS = static_cast<unsigned char>(input->GetKeyState('S'));
    inputPacket.stateD = static_cast<unsigned char>(input->GetKeyState('D'));
    inputPacket.stateShift = static_cast<unsigned char>(input->GetKeyState(VK_SHIFT));

    bool shouldSend = inputPacket.stateW != inputCache.stateW;
    shouldSend |= (inputPacket.stateA != inputCache.stateA);
    shouldSend |= (inputPacket.stateS != inputCache.stateS);
    shouldSend |= (inputPacket.stateD != inputCache.stateD);
    shouldSend |= (inputPacket.stateShift != inputCache.stateShift);
    shouldSend |= this->sendPacketTimer > this->sendPacketTimeInterval;
    if ( shouldSend )
    {
        this->sendPacketTimer = 0.0f;
        inputPacket.rotation = this->characterTransform->GetRotation();
        this->SendPacket(&inputPacket);
        inputCache = inputPacket;
    }
}

void KG::Component::CPlayerControllerComponent::OnChangeBulletCount(int count)
{
    constexpr static KG::Utill::hashType ids[10] = {
        "digit_0"_id, "digit_1"_id , "digit_2"_id , "digit_3"_id  ,"digit_4"_id,
        "digit_5"_id , "digit_6"_id , "digit_7"_id , "digit_8"_id  ,"digit_9"_id
    };
    int digit_0_value = count % 10;
    int digit_1_value = count / 10;
    digit_0->material2D.materialId = ids[digit_0_value];
    digit_0->ReloadRender();
    digit_1->material2D.materialId = ids[digit_1_value];
    digit_1->ReloadRender();
}

void KG::Component::CPlayerControllerComponent::ProcessMove(float elapsedTime)
{
    auto input = KG::Input::InputManager::GetInputManager();
    bool forwardInput = false;
    bool rightInput = false;
    float speed = input->IsTouching(VK_LSHIFT) ? 6.0f : 2.0f;
    speed *= speedValue;
    if ( input->IsTouching('W') )
    {
        this->forwardValue += inputRatio * +1 * elapsedTime;
        forwardInput = true;
    }
    if ( input->IsTouching('S') )
    {
        this->forwardValue += inputRatio * -1 * elapsedTime;
        forwardInput = true;
    }
    if ( input->IsTouching('D') )
    {
        this->rightValue += inputRatio * +1 * elapsedTime;
        rightInput = true;
    }
    if ( input->IsTouching('A') )
    {
        this->rightValue += inputRatio * -1 * elapsedTime;
        rightInput = true;
    }

    if ( !forwardInput )
    {
        if ( abs(forwardValue) > this->inputMinimum )
        {
            forwardValue += inputRetRatio * ((forwardValue > 0) ? -1 : 1) * elapsedTime;
        }
        else
        {
            forwardValue = 0.0f;
        }
    }

    if ( !rightInput )
    {
        if ( abs(rightValue) > this->inputMinimum )
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

    if ( abs(this->forwardValue) >= this->inputMinimum )
    {
        this->characterTransform->Translate(this->characterTransform->GetLook() * speed * elapsedTime * this->forwardValue);
    }
    if ( abs(this->rightValue) >= this->inputMinimum )
    {
        this->characterTransform->Translate(this->characterTransform->GetRight() * speed * elapsedTime * this->rightValue);
    }
}

void KG::Component::CPlayerControllerComponent::ProcessMoveAnimation(float elapsedTime)
{
    if ( physics == nullptr )
    {
        this->physics = this->gameObject->GetComponent<DynamicRigidComponent>();
    }

    if ( this->forwardValue >= this->inputMinimum )
    {
        //앞
        if ( this->rightValue >= this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fr, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook(), 20);
                // physics->AddForce(this->characterTransform->GetRight(), 20);
            }
        }
        else if ( this->rightValue <= -this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook(), 20);
                // auto l = this->characterTransform->GetRight() * -1;
                // l.y = 0;
                // physics->AddForce(l, 20);
            }
        }
        else
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_f, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook(), 20);
            }
        }
    }
    else if ( this->forwardValue <= -this->inputMinimum )
    {
        //뒤
        if ( this->rightValue >= this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_br, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook() * -1, 20);
                // physics->AddForce(this->characterTransform->GetRight(), 20);
            }
        }
        else if ( this->rightValue <= -this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_bl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_fl, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook() * -1, 20);
                // auto l = this->characterTransform->GetRight() * -1;
                // l.y = 0;
                // physics->AddForce(l, 20);
            }
        }
        else
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_b, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetLook() * -1, 20);
            }
        }
    }
    else
    {
        if ( this->rightValue >= this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_r, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // physics->AddForce(this->characterTransform->GetRight(), 20);
            }
        }
        else if ( this->rightValue <= -this->inputMinimum )
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::walk_l, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
            if ( physics != nullptr )
            {
                // auto l = this->characterTransform->GetRight() * -1;
                // l.y = 0;
                // physics->AddForce(l, 20);
            }
        }
        else
        {
            this->characterAnimation->ForceChangeAnimation(SoldierAnimSet::standing, 0, ANIMSTATE_PLAYING, walkBlendingDuration, ANIMLOOP_INF);
        }
    }
}

void KG::Component::CPlayerControllerComponent::ProcessShoot(float elapsedTime)
{
    auto input = KG::Input::InputManager::GetInputManager();
    if ( input->IsTouching(VK_LBUTTON) && input->GetMouseCapture() )
    {
        this->TryShoot(elapsedTime);
    }
}

void KG::Component::CPlayerControllerComponent::ProcessMouse(float elapsedTime)
{
    auto input = KG::Input::InputManager::GetInputManager();
    if ( input->IsTouching(VK_RBUTTON) || input->GetMouseCapture() )
    {
        auto delta = input->GetDeltaMousePosition();
        if ( delta.x )
        {
            this->characterTransform->RotateAxis(Math::up, delta.x * 0.3f);
            if ( physics != nullptr )
            {
                physics->SetRotation(this->characterTransform->GetRotation());
            }
        }
        if ( delta.y )
        {
            auto euler = this->cameraTransform->GetEulerDegree();
            if ( delta.y < 0 && euler.x > -85.0f || delta.y > 0 && euler.x < 80.0f )
            {
                this->cameraTransform->RotateAxis(Math::right, delta.y * 0.3f);
                if ( physics != nullptr )
                {
                    physics->SetRotation(this->characterTransform->GetRotation());
                }
            }
        }
    }
}

void KG::Component::CPlayerControllerComponent::TryShoot(float elapsedTime)
{
    if ( this->bulletCount <= 0 )
    {
        this->TryReload(elapsedTime);
        return;
    }
    if ( this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::fire || this->vectorAnimation->GetCurrentPlayingAnimationTime() > this->bulletRepeatTime )
    {
        DebugNormalMessage("Current Vector Anim Not Fire");
        this->particleGen->EmitParticle(KG::Utill::HashString("Muzzle"_id));
        this->vectorAnimation->SetAnimation(VectorAnimSet::fire, 0, 1, 1.5f);
        this->bulletCount -= 1;
        OnChangeBulletCount(this->bulletCount);
        if ( this->sound )
        {
            int randSound = KG::Math::RandomInt(VECTOR_SOUND::FIRE_1, VECTOR_SOUND::FIRE_4);
            this->sound->PlayEffectiveSound(randSound);
        }
        Packet::CS_FIRE p = { };
        p.origin = this->cameraTransform->GetWorldPosition();
        p.direction = this->cameraTransform->GetWorldLook();
        // p.direction = this->characterTransform->GetWorldLook();
        p.distance = 1000;
        this->SendPacket(&p);

        auto comp = this->physics->GetScene()->QueryRaycastResult(p.origin, p.direction, p.distance);
        if ( comp.targetRigid )
        {
            this->particleGen->EmitParticle(KG::Utill::HashString("Spark"_id), comp.hitPosition, comp.normal * 2);
            {
                auto start = this->particleGen->GetGameObject()->GetTransform()->GetWorldPosition();
                auto end = comp.hitPosition;
                float speed = 50.0f;
                auto direction = KG::Math::Vector3::Normalize(end - start) * speed;
                float lifeTime = comp.distance / speed;
                this->particleGen->EmitParticle(KG::Utill::HashString("BulletLine"_id), start, direction, lifeTime);
            }
        }
        //else 
        {
            auto start = this->particleGen->GetGameObject()->GetTransform()->GetWorldPosition();
            float speed = 50.0f;
            auto direction = this->cameraTransform->GetWorldLook() * speed;
            this->particleGen->EmitParticle(KG::Utill::HashString("BulletLine"_id), start, direction, 10.0f);
        }
    }
}

void KG::Component::CPlayerControllerComponent::TryReload(float elapsedTime)
{
    if ( !(this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::fire && this->vectorAnimation->GetCurrentPlayingAnimationTime() <= this->bulletRepeatTime)
        && this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::reload
        && this->vectorAnimation->GetCurrentPlayingAnimationId() != VectorAnimSet::reload_e
        )
    {
        if ( this->bulletCount > 0 )
        {
            this->reloadFlag = true;
            this->vectorAnimation->SetAnimation(VectorAnimSet::reload, 0, 1, 1.0f);
            if (this->sound)
                this->sound->PlayEffectiveSound(VECTOR_SOUND::RELOAD);
        }
        else
        {
            this->reloadFlag = true;
            this->vectorAnimation->SetAnimation(VectorAnimSet::reload_e, 0, 1, 1.0f);
            if (this->sound)
                this->sound->PlayEffectiveSound(VECTOR_SOUND::RELOAD_EMPTY);
        }
        Packet::CS_RELOAD packet;
        this->SendPacket(&packet);
    }
}

bool KG::Component::CPlayerControllerComponent::CheckReloading()
{
    if ( this->reloadFlag && (this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::reload || this->vectorAnimation->GetCurrentPlayingAnimationId() == VectorAnimSet::reload_e) )
    {
        if ( this->vectorAnimation->GetCurrentPlayingAnimationTime() >= this->vectorAnimation->GetCurrentPlayingAnimationDuration() * 0.8f )
        {
            this->reloadFlag = false;
            this->bulletCount = 30;
            OnChangeBulletCount(this->bulletCount);
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        this->reloadFlag = false;
        return false;
    }
}
