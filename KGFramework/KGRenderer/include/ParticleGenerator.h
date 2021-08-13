#pragma once
//#include <vector>
#include <array>
#include <map>
#include <future>
#include "ParticleEmitterComponent.h"
#include "KGGraphicBuffer.h"
#include "KGShader.h"
#include "KGGeometry.h"
namespace KG::Renderer
{
	class ParticleGenerator
	{
		static constexpr size_t MAX_PARTICLE_COUNT = 10'000;
		static constexpr float CLEAN_CYCLE_TIME = 2.5f;

        KG::Renderer::Geometry* particleGeometry = nullptr;

        std::map<KG::Utill::HashString, KG::Component::ParticleDesc> particleDescs;

		std::array<KG::Component::ParticleData, MAX_PARTICLE_COUNT> transparentParticles;
		KG::Renderer::Shader* transparentParticleShader = nullptr;
		KG::Renderer::KGRenderJob* transparentRenderJob = nullptr;
		size_t currentTransparentParticleCount = 0;

		std::array<KG::Component::ParticleData, MAX_PARTICLE_COUNT> addParticles;
		KG::Renderer::Shader* addParticleShader = nullptr;
		KG::Renderer::KGRenderJob* addRenderJob = nullptr;
		size_t currentAddParticleCount = 0;

		float cleanTimer = 0.0f;
		size_t GetEmptyIndexFromAddParticles() const;
		size_t GetEmptyIndexFromTransparentParticles() const;
		void DestroyExpired();
        std::future<bool> copyAddFuture;
        std::future<bool> copyTransparentFuture;
    public:
        void PreRenderStart();
        void PreRenderCheck();
        void Initialize();
        void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3& position);
        void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& baseSpeed);
        void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& baseSpeed, float lifeTime);
        void EmitParticle(const KG::Component::ParticleData& desc, bool autoFillTime, KG::Component::ParticleType type);
		void Update(float elapsedTime);
        void AddParticleDesc(const KG::Utill::HashString& id, KG::Component::ParticleDesc desc);
		static bool isExpired(const KG::Component::ParticleData& desc);
	};
};