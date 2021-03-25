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


		// ������ ������Ʈ���� ����� �� ��
		// 1. simulate() = Advance()
		// 2. fetchResult() - rigid�� ����
		// 3. game object�� ����
		bool Advance(float timeElapsed);

		// addActor���� ����� �� ��
		// 1. ���͸��� ����
		// 2. ������ ���� �� ����
		// 3. ���� ���� �߰�
		void AddActor(DirectX::XMFLOAT3 position, float width, float height, float depth);
		void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth);

		// �ӽ� �����
		void AddFloor(float height);

		// ���߿� ������Ʈ�� ����� �� �κ�
		std::vector<physx::PxRigidDynamic*> rigid;
		void Move(DirectX::XMFLOAT3 vector);
		DirectX::XMFLOAT3 GetPosition();
		void SetRotation(DirectX::XMFLOAT4 quat); // ĳ���� ��Ʈ�ѷ�
	};
}
