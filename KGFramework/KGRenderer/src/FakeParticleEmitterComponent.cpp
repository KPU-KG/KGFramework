#include "FakeParticleEmitterComponent.h"
#include "..\include_fake\FakeParticleEmitterComponent.h"

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

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed, float lifeTime)
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id)
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position)
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed)
{
}

void KG::Component::FakeParticleEmitterComponent::EmitParticle(const ParticleData& desc, bool autoFillTime, ParticleType type)
{
}

void KG::Component::FakeParticleEmitterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
}

void KG::Component::FakeParticleEmitterComponent::OnPreRender()
{
}

void KG::Component::FakeParticleEmitterComponent::AddParticleDesc(const KG::Utill::HashString& id, const ParticleDesc& desc)
{
}
