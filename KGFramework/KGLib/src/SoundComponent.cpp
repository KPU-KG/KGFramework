#include "SoundComponent.h"
#include "fmod.hpp"

void KG::Component::SoundComponent::LinkSystem(FMOD::System* system, FMOD::Channel* channel)
{
	this->system = system;
	this->channel = channel;
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
