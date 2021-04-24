#include "ParticleEmitterComponent.h"
#include "KGDXRenderer.h"
#include "Transform.h"

using namespace DirectX;
using namespace KG::Math::Literal;

void KG::Component::ParticleEmitterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	this->transform = gameObject->GetComponent<TransformComponent>();
}

KG::Component::ParticleEmitterComponent::ParticleEmitterComponent()
	:
	baseDeltaPositionProp("baseDeltaPosition", baseDeltaPosition),
	rangeDeltaPositionProp("rangeDeltaPosition", rangeDeltaPosition),
	baseSizeProp("baseSize", baseSize),
	rangeSizeProp("rangeSize", rangeSize),
	baseSpeedProp("baseSpeed", baseSpeed),
	rangeSpeedProp("rangeSpeed", rangeSpeed),
	colorProp("color", color),
	baselifeTimeProp("baselifeTime", baselifeTime),
	rangelifeTimeProp("rangelifeTime", rangelifeTime),
	baseRotationProp("baseRotation", baseRotation),
	rangeRotationProp("rangeRotation", rangeRotation),
	baseRotationSpeedProp("baseRotationSpeed", baseRotationSpeed),
	ramgeRotationSpeedProp("ramgeRotationSpeed", rangeRotationSpeed),
	emitPerSecondProp("emitPerSecond", emitPerSecond),
	baseEmitCountSecondProp("baseEmitCountSecond", baseEmitCount),
	rangeEmitCountSecondProp("rangeEmitCountSecond", rangeEmitCount),
	particleMaterialProp("particleMaterial", particleMaterial)
{

	this->baseSize = XMFLOAT2(1,1);
	color = XMFLOAT4(1,1,1,1);

	this->baselifeTime = 10.0f;
	this->baseEmitCount = 1;
}

void KG::Component::ParticleEmitterComponent::OnRender(ID3D12GraphicsCommandList* commadList)
{
}

void KG::Component::ParticleEmitterComponent::OnPreRender()
{
}

void KG::Component::ParticleEmitterComponent::EmitParticle()
{
	int count = KG::Math::RandomInt(baseEmitCount - rangeEmitCount, baseEmitCount + rangeEmitCount);
	for ( int i = 0; i < count; i++ )
	{
		ParticleDesc desc;
		XMFLOAT3 deltaPosition = KG::Math::RandomVector3(baseDeltaPosition - rangeDeltaPosition, baseDeltaPosition + rangeDeltaPosition);
		desc.position = this->transform->GetWorldPosition() + deltaPosition;
		desc.size = KG::Math::RandomVector2(baseSize - rangeSize, baseSize + rangeSize);
		desc.speed = KG::Math::RandomVector3(baseSpeed - rangeSpeed, baseSpeed + rangeSpeed);
		desc.color = color;
		desc.lifeTime = KG::Math::RandomFloat(baselifeTime - rangelifeTime, baselifeTime + rangelifeTime);
		desc.rotation = KG::Math::RandomFloat(baseRotation - rangeRotation, baseRotation + rangeRotation);
		desc.rotationSpeed = KG::Math::RandomFloat(baseRotationSpeed - rangeRotationSpeed, baseRotationSpeed + rangeRotationSpeed);
		this->EmitParticle(desc, true);
	}
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const ParticleDesc& desc, bool autoFillTime)
{
	KG::Renderer::KGDXRenderer::GetInstance()->EmitParticle(desc, autoFillTime);
}

UINT KG::Component::ParticleEmitterComponent::GetParticleMaterialIndex(const KG::Utill::HashString& id) const
{
	return KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(id);
}

void KG::Component::ParticleEmitterComponent::Update(float elapsedTime)
{
	if ( this->emitPerSecond )
	{
		emitTimer += elapsedTime;
		while ( emitTimer < 1.0f / this->emitPerSecond )
		{
			this->EmitParticle();
			emitTimer -= 1.0f / this->emitPerSecond;
		}
	}
}

void KG::Component::ParticleEmitterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::ParticleEmitterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::ParticleEmitterComponent::OnDrawGUI()
{
	static ParticleDesc desc;
	static DirectX::XMFLOAT3 minSpeed;
	static DirectX::XMFLOAT3 maxSpeed;
	if ( ImGui::ComponentHeader<ParticleEmitterComponent>() )
	{
		if ( ImGui::CollapsingHeader("ParticleDesc", ImGuiTreeNodeFlags_DefaultOpen) )
		{
			baseDeltaPositionProp.OnDrawGUI();
			rangeDeltaPositionProp.OnDrawGUI();
			baseSpeedProp.OnDrawGUI();
			rangeSpeedProp.OnDrawGUI();
			colorProp.OnDrawGUI();
			baselifeTimeProp.OnDrawGUI();
			rangelifeTimeProp.OnDrawGUI();
			baseRotationProp.OnDrawGUI();
			rangeRotationProp.OnDrawGUI();
			baseRotationSpeedProp.OnDrawGUI();
			ramgeRotationSpeedProp.OnDrawGUI();
			emitPerSecondProp.OnDrawGUI();
			baseEmitCountSecondProp.OnDrawGUI();
			rangeEmitCountSecondProp.OnDrawGUI();
		}
		if ( ImGui::CollapsingHeader("ParticleMaterial", ImGuiTreeNodeFlags_DefaultOpen) )
		{
			particleMaterialProp.OnDrawGUI();
			if ( ImGui::SmallButton("Set") )
			{
				this->SetParticleMaterial(this->particleMaterial);
			}
		}
	}
	return false;
}

void KG::Component::ParticleEmitterComponent::SetParticleMaterial(const KG::Utill::HashString& materialId)
{
	this->particleMaterial = materialId;
	this->particleMaterialIndex = this->GetParticleMaterialIndex(materialId);
}
