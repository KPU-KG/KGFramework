#pragma once
#include "MathHelper.h"
#include <functional>
#include "IComponent.h"

constexpr const unsigned int SOUND_EFF_SHOT = 0;

namespace FMOD
{
	class System;
	class Channel;
	class Sound;
}

namespace KG::Component
{
	// 사운드 등록하는거
	// 재생하는거
	// 루프? - 이건 나중에 (배경음악)
	// 
	class ISoundComponent : public IComponent {
	public:

		virtual void LinkSystem(FMOD::System* system, FMOD::Channel* channel) = 0;
		virtual void Update(float timeElapsed) override = 0;
		virtual void PlayEffectiveSound(UINT soundId, int loop = 1) = 0;
		virtual void PlayBackgroundSound(UINT soundId, int loop = -1) = 0;
		virtual void RegisterSound(FMOD::Sound* sound, UINT soundId) = 0;
		// virtual void RegisterSound(const char* path, UINT soundId) = 0;
	};

	REGISTER_COMPONENT_ID(ISoundComponent);
}