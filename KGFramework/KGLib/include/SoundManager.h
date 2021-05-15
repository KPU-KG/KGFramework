#pragma once
#include "SoundSystem.h"
#include "ISoundManager.h"
#include "Debug.h"
#include <unordered_map>

enum FMOD_RESULT;

namespace FMOD
{
	class System;
	class Channel;
	class Sound;
}

namespace KG::Sound 
{
	enum class SoundType {
		EFFECTIVE = 0,
		BACKGROUND
	};

	class SoundManager : public ISoundManager {
	private:
		FMOD::System* system;
		std::unordered_map<unsigned int, FMOD::Sound*> sounds;
		FMOD::Channel* channel;
		unsigned int version;
		void* extradriverdata = 0;
		// 채널을 일단 하나만 만들어 놓고
		// 나중에 추가하든 해야할듯
		// 근데 어차피 나중에 사운드 트레이싱 할거라
		// 다 뒤집을듯 ㅋㅋ
		KG::System::SoundSystem* soundSystem = nullptr;
		void ErrorCheck(FMOD_RESULT result);
	public:
		~SoundManager();
		void Initialize();
		void RegisterSound(const char* path, SoundType type, unsigned int id);
		void Update(float timeElapsed);
		KG::Component::ISoundComponent* GetNewSoundComponent();
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider);
		FMOD::Sound* GetSound(unsigned int id);
		FMOD::System* GetFmodSystem();
		FMOD::Channel* GetChannel();
	};
}