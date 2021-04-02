#pragma once
#include "IPhysicsComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"

namespace KG::Core
{
	class GameObject;
};

namespace physx
{
	class PxPhysics;
	class PxRigidDynamic;
	class PxRigidStatic;
}
namespace KG::Component
{
	class TransformComponent;

	// enum RigidType {
	// 	DynamicRigid,
	// 	StaticRigid
	// };

	// struct RigidData {
	// 	DirectX::XMFLOAT3 position;
	// 	DirectX::XMFLOAT3 scale;
	// 	DirectX::XMFLOAT3 offset;
	// 	// bool isKinetic;
	// 	// bool angleLockX;
	// 	// bool angleLockY;
	// 	// bool angleLockZ;
	// 
	// 	// collision box - pos / w / h / z / offset pos
	// 	// kinetic / dynamic
	// 	// angle lock x / y / z
	// };

	// 1. physics ������Ʈ �ϳ��� ���� ����� on / off �������??
	// 2. ��Ʈ���� ���� �ٸ� ������Ʈ��??
	// �־�� �� ��ɵ�
	/// 1. �ݸ��� �ڽ�
	//    - position / width / height / depth
	//    - offset
	// 2. dynamic ����
	//    - PxRigidDynamic*
	//
	// 3. static ����
	//	  - PxRigidStatic*


	// ������ ��ǥ 2021.3.27
	// ���� ������Ʈ ���� ���� ����

	struct CollisionBox {
		DirectX::XMFLOAT3 center;
		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT3 offset;
		CollisionBox() {
			center = { 0,0,0 };
			scale = { 1,1,1 };
			offset = { 0,0,0 };
		}
	};

	class DLL DynamicRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox				collisionBox;
		TransformComponent*			transform;		// target transform
		physx::PxRigidDynamic*		actor;			// rigid 
		// RigidData					rigid{};
		bool						apply = false;

		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		DynamicRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		// void SetCollisionBox(DirectX::XMFLOAT3& position, DirectX::XMFLOAT3 scale, DirectX::XMFLOAT3 offset = { 0,0,0 });
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetActor(physx::PxRigidDynamic* actor);
	private:
		// ������ �ڵ� (���� ����)
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>	positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>	scaleProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>	offsetProp;
		KG::Core::SerializableProperty<bool>				applyProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	class DLL StaticRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox collisionBox;
		TransformComponent* transform;
		physx::PxRigidStatic* actor;
		bool apply = false;
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		StaticRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetActor(physx::PxRigidStatic* actor);
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> scaleProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3> offsetProp;
		KG::Core::SerializableProperty<bool> applyProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	// �������� �浹�� �����ؾ��� - �Ѿ� �ǰ�������

	// class DLL PhysicsComponent : public IPhysicsComponent 
	// {
	// protected:
	// 	virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	// public:
	// 	virtual void Update(float timeElapsed) override;
	// };

	REGISTER_COMPONENT_ID(DynamicRigidComponent);
	REGISTER_COMPONENT_ID(StaticRigidComponent);
}