#pragma once
#include <vector>
#include <DirectXCollision.h>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IParticleEmitterComponent.h"

namespace KG::Component
{
	class FakeParticleEmitterComponent : public IParticleEmitterComponent
	{
    public:
        // IParticleEmitterComponent을(를) 통해 상속됨
        virtual void OnCreate(KG::Core::GameObject* gameObject) override;
        virtual void OnPreRender() override;
        virtual void EmitParticle() override;
        virtual void EmitParticle(const ParticleDesc& desc, bool autoFillTime) override;
        virtual void Update(float elapsedTime) override;
        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;
        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;
        virtual bool OnDrawGUI() override;
        virtual void SetParticleMaterial(const KG::Utill::HashString& materialId) override;
    };
	REGISTER_COMPONENT_ID_REPLACE(FakeParticleEmitterComponent, IParticleEmitterComponent);
};
