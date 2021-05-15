#include "SoundManager.h"
#include "fmod_errors.h"
#include "fmod.hpp"

FMOD_RESULT result;
// 채널을 일단 하나만 만들어 놓고
// 나중에 추가하든 해야할듯
// 근데 어차피 나중에 사운드 트레이싱 할거라
// 다 뒤집을듯 ㅋㅋ

inline void KG::Sound::SoundManager::ErrorCheck(FMOD_RESULT result) {
	if (result != FMOD_OK) {
		DebugNormalMessage(FMOD_ErrorString(result));
	}
}

KG::Sound::SoundManager::~SoundManager()
{
	for (auto& sound : sounds) {
		sound.second->release();
	}
	system->close();
	system->release();

}

void KG::Sound::SoundManager::Initialize()
{
	soundSystem = new KG::System::SoundSystem();

	result = FMOD::System_Create(&system);
	ErrorCheck(result);

	result = system->getVersion(&version);
	ErrorCheck(result);

	if (version < FMOD_VERSION) {
		DebugNormalMessage("FMOD Initialize : library version dismatch");
	}

	result = system->init(32, FMOD_INIT_NORMAL, extradriverdata);
	ErrorCheck(result);
}

void KG::Sound::SoundManager::RegisterSound(const char* path, SoundType type, unsigned int id)
{
	if (sounds.count(id) != 0) {
		DebugNormalMessage("Register Sound : already exist sound");
		return;
	}
	FMOD::Sound* sound;
	result = system->createSound(path, FMOD_DEFAULT, 0, &sound);
	sounds[id] = sound;
	ErrorCheck(result);

	switch (type) {
	case KG::Sound::SoundType::EFFECTIVE:
		result = sound->setMode(FMOD_LOOP_OFF);
		ErrorCheck(result);
		break;
	case KG::Sound::SoundType::BACKGROUND:
		result = sound->setMode(FMOD_LOOP_NORMAL);
		ErrorCheck(result);
		break;
	}
}

void KG::Sound::SoundManager::Update(float timeElapsed)
{
	result = system->update();
	ErrorCheck(result);
}

KG::Component::ISoundComponent* KG::Sound::SoundManager::GetNewSoundComponent()
{
	auto* comp = soundSystem->GetNewComponent();
	return comp;
}

void KG::Sound::SoundManager::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
	soundSystem->OnPostProvider(provider);
}

FMOD::Sound* KG::Sound::SoundManager::GetSound(unsigned int id)
{
	if (sounds.count(id) == 0)
		return nullptr;
	return sounds[id];
}

FMOD::System* KG::Sound::SoundManager::GetFmodSystem()
{
	return this->system;
}

FMOD::Channel* KG::Sound::SoundManager::GetChannel()
{
	return this->channel;
}
