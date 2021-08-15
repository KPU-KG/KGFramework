#include "SoundComponent.h"
#include "fmod.hpp"
#include "math.h"
using namespace KG::Math::Literal;

void KG::Component::SoundComponent::LinkSystem(FMOD::System* system, FMOD::Channel* channel)
{
    this->system = system;
    this->channel = channel;
}

static void CopyVector(FMOD_VECTOR& target, const DirectX::XMFLOAT3& vector)
{
    target.x = vector.x;
    target.y = vector.y;
    target.z = vector.z;
}

void KG::Component::SoundComponent::Update(float timeElapsed)
{

}

void KG::Component::SoundComponent::PlayEffectiveSound(UINT soundId, int loop)
{
    if (sounds.count(soundId) == 0) {
        DebugNormalMessage("SoundComponent::PlayEffectiveSound() : invalid soundId");
        return;
    }
    sounds[soundId]->setLoopCount(loop);
    this->system->playSound(sounds[soundId], 0, false, &this->channel);
}


void KG::Component::SoundComponent::Play3DSound(UINT soundId, const DirectX::XMFLOAT3& position)
{
    if (sounds.count(soundId) == 0) {
        DebugNormalMessage("SoundComponent::Play3DSound() : invalid soundId");
        return;
    }
    FMOD_VECTOR listener_pos;
    FMOD_VECTOR listener_vel;
    CopyVector(listener_pos, position);
    listener_vel.x = 0; listener_vel.y = 0; listener_vel.z = 0;
    sounds[soundId]->setLoopCount(1);
    this->system->playSound(sounds[soundId], 0, false, &this->channel);
    this->channel->set3DAttributes(&listener_pos, &listener_vel);
    //this->channel->set
    this->channel->set3DMinMaxDistance(0, 100);
    this->channel->setVolume(0.5f);
}


void KG::Component::SoundComponent::PlayBackgroundSound(UINT soundId, int loop)
{
    // bg는 일단 나중에!
}

void KG::Component::SoundComponent::RegisterSound(FMOD::Sound* sound, UINT soundId)
{
    if (sound == nullptr) {
        DebugNormalMessage("SoundComponent::RegisterSound() : sound is nullptr");
        return;
    }
    if (sounds.count(soundId) != 0) {
        DebugNormalMessage("SoundComponent::RegisterSound() : sound is already exist");
        return;
    }
    sounds[soundId] = sound;
}


void KG::Component::SoundComponent::SetListener(KG::Component::TransformComponent* transform, float deltaTime)
{
    static XMFLOAT3 prev;
    FMOD_VECTOR listener_pos;
    FMOD_VECTOR listener_vel;
    FMOD_VECTOR listener_forward;
    FMOD_VECTOR listener_up;
    auto pos = transform->GetWorldPosition();
    auto vel = transform->GetWorldPosition() - pos;
    vel = vel * deltaTime;
    CopyVector(listener_pos, pos);
    CopyVector(listener_vel, vel);
    CopyVector(listener_forward, transform->GetWorldLook());
    CopyVector(listener_up, transform->GetWorldUp());
    system->set3DListenerAttributes(0, &listener_pos, &listener_vel, &listener_forward, &listener_up);
    system->set3DSettings(1, 5, 1);
}


// void KG::Component::SoundComponent::RegisterSound(const char* path, UINT soundId)
// {
// 	if (this->system == nullptr) {
// 		DebugNormalMessage("SoundComponent::RegisterSound() : system is nullptr");
// 		return;
// 	}
// 	if (sounds.count(soundId) != 0) {
// 		DebugNormalMessage("SoundComponent::RegisterSound() : sound is already exist");
// 		return;
// 	}
// 	this->system->createSound(path, )
// }
