#pragma once
#include <vector>
#include "IRenderComponent.h"

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
	


	struct ParticleDesc
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 speed;
		DirectX::XMFLOAT4 acc;
		DirectX::XMFLOAT4 color;
		double startTime;
		double lifeTime;
	};

	class DLL ParticleEmitterComponent : public IRenderComponent
	{
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		virtual void OnRender(ID3D12GraphicsCommandList* commadList) override;
		virtual void OnPreRender() override;
	public:
		void EmitParticle(const ParticleDesc& desc);
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID(ParticleEmitterComponent);
};
