#pragma once
#include <vector>
#include <DirectXCollision.h>
#include "IRenderComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "IParticleEmitterComponent.h"
#include "IDXRenderComponent.h"

namespace KG::Renderer
{
	struct KGRenderJob;
};

namespace KG::Component
{

	// 파티클 데스크 채워서 Emit
	// 렌더러는 사전 정의된 파티클 갯수 (5만개 예상) 의 버퍼에 할당 생성함
	// 렌더러는 Update명령 받으면 파티클을 순회 돌면서 라이프 타임 끝난 놈들을 수정

	// 그리기 제출 시에는 페이크 지오메트리로 걍 버퍼 크기 만큼 그리기 호출 함
	// 애니메이션 트랜스폼 슬롯에 파티클 이미터 버퍼 꽂기
	// 파티클 이미터 버퍼 좌표로 지오메트리 쉐이더에서 빌보드 생성
	// Emission은 아직 고려하지 않는다.
	// 일단 파티클은 쉐이더 한개 사용하게 합시다
	// 빛은 고려 X
	

	class TransformComponent;

	class ParticleEmitterComponent : public IParticleEmitterComponent, IDXRenderComponent
	{
		float emitTimer = 0.0f;
		TransformComponent* transform = nullptr;
		KG::Utill::HashString particleMaterial;
		UINT particleMaterialIndex = 0;
		bool isParticleAdd = false;


		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		ParticleEmitterComponent();
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
		virtual void OnPreRender() override;
	public:
		virtual void EmitParticle() override;
		virtual void EmitParticle(const ParticleDesc& desc, bool autoFillTime) override;
		UINT GetParticleMaterialIndex(const KG::Utill::HashString& id) const;
		bool GetParticleMaterialIsAdd(const KG::Utill::HashString& id) const;
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseDeltaPositionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeDeltaPositionProp;

		KG::Core::SerializableProperty<DirectX::XMFLOAT2> baseSizeProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT2> rangeSizeProp;


		KG::Core::SerializableProperty<DirectX::XMFLOAT3> baseSpeedProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> rangeSpeedProp;

		KG::Core::SerializableProperty<KG::Utill::Color> colorProp;

		KG::Core::SerializableProperty<float> baselifeTimeProp;
		KG::Core::SerializableProperty<float> rangelifeTimeProp;

		KG::Core::SerializableProperty<float> baseRotationProp;
		KG::Core::SerializableProperty<float> rangeRotationProp;

		KG::Core::SerializableProperty<float> baseRotationSpeedProp;
		KG::Core::SerializableProperty<float> ramgeRotationSpeedProp;

		KG::Core::SerializableProperty<float> emitPerSecondProp;

		KG::Core::SerializableProperty<float> baseEmitCountSecondProp;
		KG::Core::SerializableProperty<float> rangeEmitCountSecondProp;

		KG::Core::SerializableProperty<KG::Utill::HashString> particleMaterialProp;

	public:
		virtual void Update(float elapsedTime) override;
		virtual void SetParticleMaterial(const KG::Utill::HashString& materialId) override;
    public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID_REPLACE(ParticleEmitterComponent, IParticleEmitterComponent);
};
