#include "KGDXRenderer.h"
#include "ParticleGenerator.h"

size_t KG::Renderer::ParticleGenerator::GetEmptyIndex() const
{
	auto it = std::find_if(this->particles.begin(), this->particles.end(), [](auto& a) { return isExpired(a); });

	return size_t();
}
void KG::Renderer::ParticleGenerator::DestroyExpired()
{
}

void KG::Renderer::ParticleGenerator::EmitParticle(const KG::Component::ParticleDesc& desc)
{
	this->GetEmptyIndex();
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
