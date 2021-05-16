#include "ParticleEmitterComponent.h"
#include "ResourceContainer.h"
#include "KGDXRenderer.h"
#include "Transform.h"
#include "ParticleGenerator.h"

using namespace DirectX;
using namespace KG::Math::Literal;

void KG::Component::ParticleEmitterComponent::OnCreate(KG::Core::GameObject* gameObject)
{
	this->transform = gameObject->GetComponent<TransformComponent>();
    this->particleGenerator = KG::Renderer::KGDXRenderer::GetInstance()->GetParticleGenerator();
}

KG::Component::ParticleEmitterComponent::ParticleEmitterComponent()
	:
	baseDeltaPositionProp("baseDeltaPosition", previewParticle.baseDeltaPosition),
	rangeDeltaPositionProp("rangeDeltaPosition", previewParticle.rangeDeltaPosition),
	baseSizeProp("baseSize", previewParticle.baseSize),
	rangeSizeProp("rangeSize", previewParticle.rangeSize),
	baseSpeedProp("baseSpeed", previewParticle.baseSpeed),
	rangeSpeedProp("rangeSpeed", previewParticle.rangeSpeed),
	colorProp("color", previewParticle.color),
	baselifeTimeProp("baselifeTime", previewParticle.baselifeTime),
	rangelifeTimeProp("rangelifeTime", previewParticle.rangelifeTime),
	baseRotationProp("baseRotation", previewParticle.baseRotation),
	rangeRotationProp("rangeRotation", previewParticle.rangeRotation),
	baseRotationSpeedProp("baseRotationSpeed", previewParticle.baseRotationSpeed),
	ramgeRotationSpeedProp("ramgeRotationSpeed", previewParticle.rangeRotationSpeed),
	baseEmitCountSecondProp("baseEmitCountSecond", previewParticle.baseEmitCount),
	rangeEmitCountSecondProp("rangeEmitCountSecond", previewParticle.rangeEmitCount),
    materialIdProp("materialId", previewParticle.materialId)
{
}

void KG::Component::ParticleEmitterComponent::OnRender(ID3D12GraphicsCommandList* commadList)
{
}

void KG::Component::ParticleEmitterComponent::OnPreRender()
{
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id)
{
    this->particleGenerator->EmitParticle(id, this->gameObject->GetTransform()->GetWorldPosition());
}
void KG::Component::ParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position)
{
    this->particleGenerator->EmitParticle(id, position);
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed)
{
    this->particleGenerator->EmitParticle(id, position, baseSpeed);
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed, float lifeTime)
{
    this->particleGenerator->EmitParticle(id, position, baseSpeed, lifeTime);
}

void KG::Component::ParticleEmitterComponent::EmitParticle(const ParticleData& desc, bool autoFillTime, ParticleType type)
{
    this->particleGenerator->EmitParticle(desc, autoFillTime, type);
}

void KG::Component::ParticleEmitterComponent::AddParticleDesc(const KG::Utill::HashString& id, const ParticleDesc& desc)
{
    this->particleGenerator->AddParticleDesc(id, desc);
}


UINT KG::Component::ParticleEmitterComponent::GetParticleMaterialIndex(const KG::Utill::HashString& id) const
{
    return KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(id);
}

bool KG::Component::ParticleEmitterComponent::GetParticleMaterialIsAdd(const KG::Utill::HashString& id) const
{
	return KG::Resource::ResourceContainer::GetInstance()->LoadMaterial(id).second == "ParticleAdd"_id;
}

void KG::Component::ParticleEmitterComponent::Update(float elapsedTime)
{
	//if ( this->emitPerSecond )
	//{
	//	emitTimer += elapsedTime;
	//	while ( emitTimer < 1.0f / this->emitPerSecond )
	//	{
	//		this->EmitParticle();
	//		emitTimer -= 1.0f / this->emitPerSecond;
	//	}
	//}
}

void KG::Component::ParticleEmitterComponent::OnDataLoad(tinyxml2::XMLElement* componentElement)
{
}

void KG::Component::ParticleEmitterComponent::OnDataSave(tinyxml2::XMLElement* parentElement)
{
}

bool KG::Component::ParticleEmitterComponent::OnDrawGUI()
{
	static ParticleData desc;
	static DirectX::XMFLOAT3 minSpeed;
	static DirectX::XMFLOAT3 maxSpeed;
	if ( ImGui::ComponentHeader<ParticleEmitterComponent>() )
	{
		if ( ImGui::CollapsingHeader("ParticleDesc", ImGuiTreeNodeFlags_DefaultOpen) )
		{
			baseDeltaPositionProp.OnDrawGUI();
			rangeDeltaPositionProp.OnDrawGUI();
			baseSizeProp.OnDrawGUI();
			rangeSizeProp.OnDrawGUI();
			baseSpeedProp.OnDrawGUI();
			rangeSpeedProp.OnDrawGUI();
			colorProp.OnDrawGUI();
			baselifeTimeProp.OnDrawGUI();
			rangelifeTimeProp.OnDrawGUI();
			baseRotationProp.OnDrawGUI();
			rangeRotationProp.OnDrawGUI();
			baseRotationSpeedProp.OnDrawGUI();
			ramgeRotationSpeedProp.OnDrawGUI();
			//emitPerSecondProp.OnDrawGUI();
			baseEmitCountSecondProp.OnDrawGUI();
			rangeEmitCountSecondProp.OnDrawGUI();
			materialIdProp.OnDrawGUI();
		}
        bool emit = false;
        if ( ImGui::Button("EmitAdd"))
        {
            emit = true;
            previewParticle.type = ParticleType::Add;
        }
        if ( ImGui::Button("EmitTrans") )
        {
            emit = true;
            previewParticle.type = ParticleType::Transparent;
        }
        if( emit )
        {
            this->previewParticle.materialIndex = KG::Renderer::KGDXRenderer::GetInstance()->QueryMaterialIndex(previewParticle.materialId);
            this->EmitParticle(this->previewParticle.GetData(this->gameObject->GetTransform()->GetWorldPosition()), true, previewParticle.type);
        }
	}
	return false;
}

