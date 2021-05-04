#include "KGDXRenderer.h"
#include "ParticleGenerator.h"
#include "KGGraphicBuffer.h"
#include "KGRenderQueue.h"
#include "RootParameterIndex.h"
#include "ResourceContainer.h"
#include <iterator>
#include <algorithm>


size_t KG::Renderer::ParticleGenerator::GetEmptyIndexFromAddParticles() const
{
	auto& particles = this->addParticles;
	auto it = std::find_if(particles.begin(), particles.end(), [](auto& a) { return isExpired(a); });
	return std::distance(particles.begin(), it);
}
size_t KG::Renderer::ParticleGenerator::GetEmptyIndexFromTransparentParticles() const
{
	auto& particles = this->transparentParticles;
	auto it = std::find_if(particles.begin(), particles.end(), [](auto& a) { return isExpired(a); });
	return std::distance(particles.begin(), it);
}
void KG::Renderer::ParticleGenerator::DestroyExpired()
{
	{
		auto& particles = this->transparentParticles;
		auto& currentParticleCount = this->currentTransparentParticleCount;
		auto bound = std::partition(particles.begin(), particles.end(), [](auto& a) { return isExpired(a); });
		currentParticleCount = std::distance(particles.begin(), bound);
	}
	{
		auto& particles = this->addParticles;
		auto& currentParticleCount = this->currentAddParticleCount;
		auto bound = std::partition(particles.begin(), particles.end(), [](auto& a) { return isExpired(a); });
		currentParticleCount = std::distance(particles.begin(), bound);
	}
}

void KG::Renderer::ParticleGenerator::Initialize()
{
	this->particleGeometry = KG::Resource::ResourceContainer::GetInstance()->CreateFakeGeometry(D3D12_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_POINTLIST, 1);

	this->transparentParticleShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("ParticleTransparent"_id);
	this->transparentRenderJob = KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->transparentParticleShader, this->particleGeometry);
	this->transparentRenderJob->GetNewBuffer();


	this->addParticleShader = KG::Resource::ResourceContainer::GetInstance()->LoadShader("ParticleAdd"_id);
	this->addRenderJob = KGDXRenderer::GetInstance()->GetRenderEngine()->GetRenderJob(this->addParticleShader, this->particleGeometry);
	this->addRenderJob->GetNewBuffer();

	memset(this->addParticles.data(), 0, sizeof(this->addParticles[0]) * this->addParticles.size());
	memset(this->transparentParticles.data(), 0, sizeof(this->transparentParticles[0]) * this->transparentParticles.size());
}

void KG::Renderer::ParticleGenerator::EmitParticle(const KG::Component::ParticleDesc& desc, bool autoFillTime, bool isAdd)
{
	auto& currentParticleCount = isAdd ? this->currentAddParticleCount : this->currentTransparentParticleCount;
	auto& particles = isAdd ? this->addParticles : this->transparentParticles;
	auto index = isAdd ? this->GetEmptyIndexFromAddParticles() : this->GetEmptyIndexFromTransparentParticles();

	currentParticleCount++;
	particles[index] = desc;
	if ( autoFillTime )
	{
		particles[index].startTime = KGDXRenderer::GetInstance()->GetGameTime();
	}
}


void KG::Renderer::ParticleGenerator::EmitParticleAdd(const KG::Component::ParticleDesc& desc, bool autoFillTime)
{
	this->EmitParticle(desc, autoFillTime, true);
}

void KG::Renderer::ParticleGenerator::EmitParticleTransparent(const KG::Component::ParticleDesc& desc, bool autoFillTime)
{
	this->EmitParticle(desc, autoFillTime, false);
}

void KG::Renderer::ParticleGenerator::PreRender()
{
	{
		auto* renderJob = this->addRenderJob;
		auto& particles = this->addParticles;
		memcpy(renderJob->objectBuffer->mappedData, particles.data(), sizeof(particles[0]) * particles.size());
		renderJob->SetUpdateCount(particles.size());
		renderJob->SetVisibleSize(particles.size());
	}

	{
		auto* renderJob = this->transparentRenderJob;
		auto& particles = this->transparentParticles;
		memcpy(renderJob->objectBuffer->mappedData, particles.data(), sizeof(particles[0]) * particles.size());
		renderJob->SetUpdateCount(particles.size());
		renderJob->SetVisibleSize(particles.size());
	}
}

void KG::Renderer::ParticleGenerator::Update(float elapsedTime)
{
	//cleanTimer += elapsedTime;
	//if ( cleanTimer >= CLEAN_CYCLE_TIME )
	//{
	//	cleanTimer = 0.0f;
	//	this->DestroyExpired();
	//}
}

bool KG::Renderer::ParticleGenerator::isExpired(const KG::Component::ParticleDesc& desc)
{
	return KGDXRenderer::GetInstance()->GetGameTime() > (desc.startTime + desc.lifeTime);
}