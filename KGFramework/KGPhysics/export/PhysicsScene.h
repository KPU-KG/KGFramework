#pragma once

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // PHYSICSPART_EXPORTS

#include <DirectXMath.h>
#include <vector>

namespace physx 
{
	class PxDefaultAllocator;
	class PxDefaultErrorCallback;
	class PxFoundation;
	class PxPhysics;
	class PxDefaultCpuDispatcher;
	class PxPvd;
	class PxScene;
	class PxRigidDynamic;
}

namespace KG::Physics 
{
	// using namespace physx;
	class DLL PhysicsScene {
	private:
		physx::PxDefaultAllocator* allocator;
		physx::PxDefaultErrorCallback* errorCallback;
		physx::PxFoundation* foundation;
		physx::PxPhysics* physics;
		physx::PxDefaultCpuDispatcher* cpuDispatcher;
		physx::PxPvd* pvd;
		physx::PxScene* scene;

		float accumulator = 0.0f;
		float stepSize = 1.0f / 60.0f;
	public:
		bool Initialize();
		bool CreateScene();


		// 물리쪽 업데이트에서 해줘야 할 것
		// 1. simulate() = Advance()
		// 2. fetchResult() - rigid값 변동
		// 3. game object에 적용
		bool Advance(float timeElapsed);

		// addActor에서 해줘야 할 것
		// 1. 메터리얼 생성
		// 2. 리지드 생성 및 설정
		// 3. 씬에 엑터 추가
		void AddActor(DirectX::XMFLOAT3 position, float width, float height, float depth);
		void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth);

		// 임시 지평면
		void AddFloor(float height);

		// 나중에 컴포넌트로 빼줘야 할 부분
		std::vector<physx::PxRigidDynamic*> rigid;
		void Move(DirectX::XMFLOAT3 vector);
		DirectX::XMFLOAT3 GetPosition();
		void SetRotation(DirectX::XMFLOAT4 quat); // 캐릭터 컨트롤러
	};
}
