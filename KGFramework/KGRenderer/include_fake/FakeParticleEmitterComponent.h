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

        virtual void AddParticleDesc(const KG::Utill::HashString& id, const ParticleDesc& desc) override;

        virtual void EmitParticle(const KG::Utill::HashString& id) override;

        virtual void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position) override;

        virtual void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed) override;

        virtual void EmitParticle(const ParticleData& desc, bool autoFillTime, ParticleType type) override;

        virtual void Update(float elapsedTime) override;

        virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) override;

        virtual void OnDataSave(tinyxml2::XMLElement* parentElement) override;

        virtual bool OnDrawGUI() override;


        // IParticleEmitterComponent을(를) 통해 상속됨
        virtual void EmitParticle(const KG::Utill::HashString& id, const DirectX::XMFLOAT3 position, const DirectX::XMFLOAT3 baseSpeed, float lifeTime) override;

    };
	REGISTER_COMPONENT_ID_REPLACE(FakeParticleEmitterComponent, IParticleEmitterComponent);
};
