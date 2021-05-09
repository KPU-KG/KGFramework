#include "FakeParticleEmitterComponent.h"

void KG::Component::FakeParticleEmitterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeParticleEmitterComponent::OnPreRender()
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle()
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const ParticleDesc& desc, bool autoFillTime)
{
}

void KG::Component::FakeParticleEmitterComponent::Update(float elapsedTime)
{
}

void KG::Component::FakeParticleEmitterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::FakeParticleEmitterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::FakeParticleEmitterComponent::OnDrawGUI()
{
    return false;
}

void KG::Component::FakeParticleEmitterComponent::SetParticleMaterial(const KG::Utill::HashString& materialId)
{
}
