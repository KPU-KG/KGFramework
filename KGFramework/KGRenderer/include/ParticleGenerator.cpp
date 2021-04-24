#include "KGDXRenderer.h"
#include "ParticleGenerator.h"
#include "KGGraphicBuffer.h"
#include "KGRenderQueue.h"
#include "RootParameterIndex.h"
#include "ResourceContainer.h"
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

void KG::Renderer::ParticleGenerator::PreRender()
{
	memcpy(this->renderJob->objectBuffer, this->particles.data(), this->currentParticleCount);
}

void KG::Renderer::ParticleGenerator::Initialize()
{
	this->particleShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("ParticleShader"_id);
	this->particleGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST, 1);
	this->renderJob =  KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->particleShader, this->particleGeometry);
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
