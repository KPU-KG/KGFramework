#pragma once
#include "MathHelper.h"
#include <functional>
#include "IComponent.h"
#include "Transform.h"

constexpr unsigned int SOUND_EFF_SHOT = 0;
constexpr unsigned int SOUND_EFF_SHOT_3D = 0;
constexpr unsigned int SOUND_EFF_RELOAD = 1;

struct VECTOR_SOUND
{
    static constexpr unsigned int DRAW = 2;

    static constexpr unsigned int FIRE_1 = 3;
    static constexpr unsigned int FIRE_2 = 4;
    static constexpr unsigned int FIRE_3 = 5;
    static constexpr unsigned int FIRE_4 = 6;

    static constexpr unsigned int RELOAD = 7;
    static constexpr unsigned int RELOAD_EMPTY = 8;

    static constexpr unsigned int FIRE_1_3D = 11;
    static constexpr unsigned int FIRE_2_3D = 12;
    static constexpr unsigned int FIRE_3_3D = 13;
    static constexpr unsigned int FIRE_4_3D = 14;
};

struct ENEMY_SOUND
{
	static constexpr unsigned int LAUNCH = 9;
	static constexpr unsigned int EXPLOSION = 10;
};



namespace FMOD
{
	class System;
	class Channel;
	class Sound;
}

namespace KG::Component
{
	// ���� ����ϴ°�
	// ����ϴ°�
	// ����? - �̰� ���߿� (�������)
	// 
	class ISoundComponent : public IComponent {
	public:
		virtual void LinkSystem(FMOD::System* system, FMOD::Channel* channel) = 0;
		virtual void Update(float timeElapsed) override = 0;
		virtual void PlayEffectiveSound(UINT soundId, int loop = 1) = 0;
		virtual void PlayBackgroundSound(UINT soundId, int loop = -1) = 0;
		virtual void RegisterSound(FMOD::Sound* sound, UINT soundId) = 0;
        virtual void Play3DSound(UINT soundId, const DirectX::XMFLOAT3& position) {};
        virtual void SetListener(KG::Component::TransformComponent* transform, float deltaTime) {};
		// virtual void RegisterSound(const char* path, UINT soundId) = 0;
	};

	REGISTER_COMPONENT_ID(ISoundComponent);
}