#pragma once
#include "ISoundComponent.h"
#include <unordered_map>

namespace KG::Component
{
	class SoundComponent : public ISoundComponent {
		FMOD::System* system;
		FMOD::Channel* channel;
		std::unordered_map<unsigned int, FMOD::Sound*> sounds;
	public:
		virtual void LinkSystem(FMOD::System* system, FMOD::Channel* channel) override;
		virtual void Update(float timeElapsed) override;
		virtual void PlayEffectiveSound(UINT soundId, int loop = 1) override;
		virtual void PlayBackgroundSound(UINT soundId, int loop = -1) override;
		virtual void RegisterSound(FMOD::Sound* sound, UINT soundId) override;
		// virtual void RegisterSound(const char* path, UINT soundId) override;
	};

	REGISTER_COMPONENT_ID(SoundComponent);
}