#pragma once
//#include <vector>
#include <array>
#include "ParticleEmitterComponent.h"
#include "KGGraphicBuffer.h"
#include "KGShader.h"
#include "KGGeometry.h"
namespace KG::Renderer
{
	class ParticleGenerator
	{
		static constexpr size_t MAX_PARTICLE_COUNT = 10'0000;
		static constexpr float CLEAN_CYCLE_TIME = 2.5f;

		KG::Renderer::Geometry* particleGeometry = nullptr;

		std::array<KG::Component::ParticleDesc, MAX_PARTICLE_COUNT> transparentParticles;
		KG::Renderer::Shader* transparentParticleShader = nullptr;
		KG::Renderer::KGRenderJob* transparentRenderJob = nullptr;
		size_t currentTransparentParticleCount = 0;

		std::array<KG::Component::ParticleDesc, MAX_PARTICLE_COUNT> addParticles;
		KG::Renderer::Shader* addParticleShader = nullptr;
		KG::Renderer::KGRenderJob* addRenderJob = nullptr;
		size_t currentAddParticleCount = 0;

		float cleanTimer = 0.0f;
		size_t GetEmptyIndexFromAddParticles() const;
		size_t GetEmptyIndexFromTransparentParticles() const;
		void DestroyExpired();
		void EmitParticle(const KG::Component::ParticleDesc& desc, bool autoFillTime, bool isAdd);
	public:
		void PreRender();
		void Initialize();
		void EmitParticleAdd(const KG::Component::ParticleDesc& desc, bool autoFillTime);
		void EmitParticleTransparent(const KG::Component::ParticleDesc& desc, bool autoFillTime);
		void Update(float elapsedTime);
		static bool isExpired(const KG::Component::ParticleDesc& desc);
	};
};