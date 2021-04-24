#include "KGDXRenderer.h"
#include "ParticleGenerator.h"
#include "KGGraphicBuffer.h"
#include <iterator>
#include <algorithm>

size_t KG::Renderer::ParticleGenerator::GetEmptyIndex() const
{
	auto it = std::find_if(this->particles.begin(), this->particles.end(), [](auto& a) { return isExpired(a); });
	return std::distance(this->particles.begin(), it);
}
void KG::Renderer::ParticleGenerator::DestroyExpired()
{
	auto bound = std::partition(this->particles.begin(), this->particles.end(), [](auto& a) { return isExpired(a); });
	currentParticleCount = std::distance(this->particles.begin(), bound);
}

void KG::Renderer::ParticleGenerator::Initialize()
{
	this->uploadBuffer.Initialize(KGDXRenderer::GetInstance()->GetD3DDevice(), MAX_PARTICLE_COUNT);
	this->uploadBuffer.resource->Map(0, nullptr, (void**)&this->particles);

	//KGDXRenderer::GetInstance()->GetD3DDevice()->
}

void KG::Renderer::ParticleGenerator::EmitParticle(const KG::Component::ParticleDesc& desc, bool autoFillTime)
{
	auto index = this->GetEmptyIndex();
	currentParticleCount++;
	this->particles[index] = desc;
	if ( autoFillTime )
	{
		this->particles[index].startTime = KGDXRenderer::GetInstance()->GetGameTime();
	}
}

void KG::Renderer::ParticleGenerator::Update(float elapsedTime)
{
	cleanTimer += elapsedTime;
	if ( cleanTimer >= CLEAN_CYCLE_TIME )
	{
		cleanTimer = 0.0f;
		this->DestroyExpired();
	}
}

bool KG::Renderer::ParticleGenerator::isExpired(const KG::Component::ParticleDesc& desc)
{
	return KGDXRenderer::GetInstance()->GetGameTime() < (desc.startTime + desc.lifeTime);
}
