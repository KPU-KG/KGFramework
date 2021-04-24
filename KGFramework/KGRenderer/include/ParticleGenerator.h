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

		size_t currentParticleCount = 0;
		std::array<KG::Component::ParticleDesc, MAX_PARTICLE_COUNT> particles;
		KG::Renderer::Shader* particleShader = nullptr;
		KG::Renderer::Geometry* particleGeometry = nullptr;
		KG::Renderer::KGRenderJob* renderJob = nullptr;

		float cleanTimer = 0.0f;
		size_t GetEmptyIndex() const;
		void DestroyExpired();
	public:
		void PreRender();
		void Initialize();
		void EmitParticle(const KG::Component::ParticleDesc& desc, bool autoFillTime);
		void Update(float elapsedTime);
		static bool isExpired(const KG::Component::ParticleDesc& desc);
	};
};