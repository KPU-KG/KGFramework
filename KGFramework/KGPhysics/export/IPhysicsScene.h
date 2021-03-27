#pragma once

#include "PhysicsComponent.h"
#include "PhysicsDesc.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // PHYSICSPART_EXPORTS

namespace KG::Physics
{
	// using namespace physx;
	class DLL IPhysicsScene {
	protected:
		PhysicsDesc desc;
		IPhysicsScene() = default;
	public:
		void SetDsec(const PhysicsDesc& desc);
		// void SetSetting();

		// bool Initialize();
		// bool CreateScene();

		virtual void Initialize(const PhysicsDesc& desc)
		{
			this->SetDsec(desc);
			this->Initialize();
		}

		virtual void Initialize() = 0;
		virtual void* GetImGUIContext() = 0;
		virtual bool Advance(float timeElapsed) = 0;

		virtual void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid) = 0;
		virtual void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth) = 0;
		virtual void AddFloor(float height) = 0;

		virtual KG::Component::DynamicRigidComponent* GetNewRenderComponent() = 0;

		// void AddStaticActor()
		// 물리쪽 업데이트에서 해줘야 할 것
		// 1. simulate() = Advance()
		// 2. fetchResult() - rigid값 변동
		// 3. game object에 적용
		// bool Advance(float timeElapsed);

		// addActor에서 해줘야 할 것
		// 1. 메터리얼 생성
		// 2. 리지드 생성 및 설정
		// 3. 씬에 엑터 추가
		// void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid);
		// void AddDynamicActor(DirectX::XMFLOAT3 position, float width, float height, float depth);
		// void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth);

		// 임시 지평면
		// void AddFloor(float height);

		// 나중에 컴포넌트로 빼줘야 할 부분
		// std::vector<physx::PxRigidDynamic*> rigid;
		// void Move(DirectX::XMFLOAT3 vector);
		// DirectX::XMFLOAT3 GetPosition();
		// void SetRotation(DirectX::XMFLOAT4 quat); // 캐릭터 컨트롤러
	};
	// DLL KG::Physics::IPhysicsScene* GetPhysiceScene();
}
