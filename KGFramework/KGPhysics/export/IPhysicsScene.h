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
		// ������ ������Ʈ���� ����� �� ��
		// 1. simulate() = Advance()
		// 2. fetchResult() - rigid�� ����
		// 3. game object�� ����
		// bool Advance(float timeElapsed);

		// addActor���� ����� �� ��
		// 1. ���͸��� ����
		// 2. ������ ���� �� ����
		// 3. ���� ���� �߰�
		// void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid);
		// void AddDynamicActor(DirectX::XMFLOAT3 position, float width, float height, float depth);
		// void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth);

		// �ӽ� �����
		// void AddFloor(float height);

		// ���߿� ������Ʈ�� ����� �� �κ�
		// std::vector<physx::PxRigidDynamic*> rigid;
		// void Move(DirectX::XMFLOAT3 vector);
		// DirectX::XMFLOAT3 GetPosition();
		// void SetRotation(DirectX::XMFLOAT4 quat); // ĳ���� ��Ʈ�ѷ�
	};
	// DLL KG::Physics::IPhysicsScene* GetPhysiceScene();
}
