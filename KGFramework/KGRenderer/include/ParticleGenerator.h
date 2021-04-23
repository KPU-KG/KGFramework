#pragma once
//#include <vector>
#include <array>
#include "ParticleEmitterComponent.h"
namespace KG::Renderer
{
	class ParticleGenerator
	{
		static constexpr size_t MAX_PARTICLE = 10'0000;
		static constexpr float CLEAN_CYCLE_TIME = 2.5f;
		std::array<KG::Component::ParticleDesc, MAX_PARTICLE> particles;
		
		float cleanTimer = 0.0f;
		size_t GetEmptyIndex() const;
		void DestroyExpired();
	public:
		void EmitParticle(const KG::Component::ParticleDesc& desc);
		void Update(float elapsedTime);
		static bool isExpired(const KG::Component::ParticleDesc& desc);
	};


};