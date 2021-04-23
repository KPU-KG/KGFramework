#include "ParticleEmitterComponent.h"

void KG::Component::ParticleEmitterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::ParticleEmitterComponent::OnRender(ID3D12GraphicsCommandList* commadList)
{
}

void KG::Component::ParticleEmitterComponent::OnPreRender()
{
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const ParticleDesc& desc)
{
}

void KG::Component::ParticleEmitterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::ParticleEmitterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::ParticleEmitterComponent::OnDrawGUI()
{
	return false;
}
