#pragma once
#include <vector>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include <DirectXCollision.h>

namespace KG::Component
{
	struct ParticleDesc
	{
		DirectX::XMFLOAT3 position;
		float rotation;
		DirectX::XMFLOAT3 speed;
		float rotationSpeed;
		DirectX::XMFLOAT2 size;
		UINT materialIndex;
		float pad1;
		DirectX::XMFLOAT4 color;
		double startTime;
		float lifeTime;
		float pad0;
	};

	class DLL IParticleEmitterComponent : public IRenderComponent
	{
		virtual void OnCreate(KG::Core::GameObject* gameObject) override = 0;
		virtual void OnPreRender() override = 0;
	public:
		DirectX::XMFLOAT3 baseDeltaPosition = DirectX::XMFLOAT3(0,0,0);
		DirectX::XMFLOAT3 rangeDeltaPosition = DirectX::XMFLOAT3(0, 0, 0);

		DirectX::XMFLOAT2 baseSize = DirectX::XMFLOAT2(1, 1);
		DirectX::XMFLOAT2 rangeSize = DirectX::XMFLOAT2(0, 0);

		DirectX::XMFLOAT3 baseSpeed = DirectX::XMFLOAT3(0, 0, 0);
		DirectX::XMFLOAT3 rangeSpeed = DirectX::XMFLOAT3(0, 0, 0);

		KG::Utill::Color color = KG::Utill::Color();

		float baselifeTime = 10;
		float rangelifeTime = 0;

		float baseRotation = 0;
		float rangeRotation = 0;

		float baseRotationSpeed = 0;
		float rangeRotationSpeed = 0;

		float emitPerSecond = 0;

		float baseEmitCount = 0;
		float rangeEmitCount = 0;


		virtual void EmitParticle() = 0;
		virtual void EmitParticle(const ParticleDesc& desc, bool autoFillTime) = 0;
	public:
		virtual void Update(float elapsedTime) override = 0;
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement) = 0;
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement) = 0;
		virtual bool OnDrawGUI() = 0;
		virtual void SetParticleMaterial(const KG::Utill::HashString& materialId) = 0;
	};
	REGISTER_COMPONENT_ID(IParticleEmitterComponent);
};
