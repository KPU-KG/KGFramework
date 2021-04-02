#pragma once

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // PHYSICSPART_EXPORTS

#include <DirectXMath.h>
#include <vector>
#include "IPhysicsScene.h"
#include "PhysicsSystem.h"

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

namespace KG::Component 
{
	class DynamicRigidComponent;
}

namespace KG::Physics 
{
	// using namespace physx;
	class DLL PhysicsScene : public IPhysicsScene {
	private:
		static inline PhysicsScene* instance = nullptr;
	protected:
		physx::PxDefaultAllocator*		allocator;
		physx::PxDefaultErrorCallback*	errorCallback;
		physx::PxFoundation*			foundation;
		physx::PxPhysics*				physics;
		physx::PxDefaultCpuDispatcher*	cpuDispatcher;
		physx::PxPvd*					pvd;
		physx::PxScene*					scene;

		float							accumulator = 0.0f;
		float							stepSize = 1.0f / 60.0f;

		struct PhysicsSystems;
		PhysicsSystems* physicsSystems = nullptr;
		bool CreateScene(float gravity);
	public:
		PhysicsScene();
		virtual void Initialize() override;
		virtual bool Advance(float timeElapsed) override;

		// �ӽ� �����
		virtual void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid) override;
		virtual void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth) override;
		virtual void AddFloor(float height) override;
		virtual KG::Component::DynamicRigidComponent* GetNewPhysicsComponent() override;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

		static PhysicsScene* GetInstance() { return instance; }
		// ������ ������Ʈ���� ����� �� ��
		// 1. simulate() = Advance()
		// 2. fetchResult() - rigid�� ����
		// 3. game object�� ����
		// bool Advance(float timeElapsed);

		// addActor���� ����� �� ��
		// 1. ���͸��� ����
		// 2. ������ ���� �� ����
		// 3. ���� ���� �߰�
		// void AddDynamicActor(DirectX::XMFLOAT3 position, float width, float height, float depth);

		

		// ���߿� ������Ʈ�� ����� �� �κ�
		// std::vector<physx::PxRigidDynamic*> rigid;
		// void Move(DirectX::XMFLOAT3 vector);
		// DirectX::XMFLOAT3 GetPosition();
		// void SetRotation(DirectX::XMFLOAT4 quat); // ĳ���� ��Ʈ�ѷ�
	};
}