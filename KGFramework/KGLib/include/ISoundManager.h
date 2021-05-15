#pragma once
#include "ComponentProvider.h"
#include <unordered_map>

namespace KG::Component
{
	class SoundComponent;
}

namespace KG::System
{
	class SoundSystem;
}

namespace FMOD
{
	class Sound;
	class Channel;
	class System;
}

namespace KG::Sound
{
	enum class SoundType;

	class ISoundManager {
	private:
		// FMOD::System* system;
		// std::unordered_map<unsigned int, FMOD::Sound*> sounds;
		// FMOD::Channel* channel;
		// FMOD_RESULT result;
		// unsigned int version;
		// void* extradriverdata = 0;
		// 채널을 일단 하나만 만들어 놓고
		// 나중에 추가하든 해야할듯
		// 근데 어차피 나중에 사운드 트레이싱 할거라
		// 다 뒤집을듯 ㅋㅋ
		// KG::System::SoundSystem* soundSystem = nullptr;
		// void ErrorCheck(FMOD_RESULT result);
	public:
		// ~SoundManager();
		virtual void Initialize() = 0;
		virtual void RegisterSound(const char* path, SoundType type, unsigned int id) = 0;
		virtual void Update(float timeElapsed) = 0;
		virtual KG::Component::ISoundComponent* GetNewSoundComponent() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;
		virtual FMOD::Sound* GetSound(unsigned int id) = 0;
		virtual FMOD::System* GetFmodSystem() = 0;
		virtual FMOD::Channel* GetChannel() = 0;
	};
}