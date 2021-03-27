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

	enum RigidType {
		DynamicRigid,
		StaticRigid
	};

	struct RigidData {
		DirectX::XMFLOAT3 position;
		float width, height, depth;
		DirectX::XMFLOAT3 offset;
		// bool isKinetic;
		// bool angleLockX;
		// bool angleLockY;
		// bool angleLockZ;

		// collision box - pos / w / h / z / offset pos
		// kinetic / dynamic
		// angle lock x / y / z
	};

	// 1. physics 컴포넌트 하나에 여러 기능은 on / off 기능으로??
	// 2. 파트별로 각자 다른 컴포넌트로??
	// 있어야 할 기능들
	/// 1. 콜리전 박스
	//    - position / width / height / depth
	//    - offset
	// 2. dynamic 액터
	//    - PxRigidDynamic*
	//
	// 3. static 액터
	//	  - PxRigidStatic*


	// 오늘의 목표 2021.3.27
	// 액터 컴포넌트 만들어서 씬에 띄우기

	struct CollisionBox {
		DirectX::XMFLOAT3 center;
		DirectX::XMFLOAT3 offset;
		float width, height, depth;
	};

	class DLL DynamicRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox				collisionBox;
		TransformComponent*			transform;		// target transform
		physx::PxRigidDynamic*		actor;		// rigid 
		RigidType					rigidType = DynamicRigid;
		RigidData					rigid{};
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		DynamicRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		void SetCollisionBox(DirectX::XMFLOAT3& position, float w, float h, float d, DirectX::XMFLOAT3 offset = { 0,0,0 });
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetActor(physx::PxRigidDynamic* actor);
	private:
		// 참고할 코드 (저장 정보)
		KG::Core::SerializableEnumProperty<RigidType>		rigidTypeProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>	positionProp;
		KG::Core::SerializableProperty<float>				widthProp;
		KG::Core::SerializableProperty<float>				heightProp;
		KG::Core::SerializableProperty<float>				depthProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>	offsetProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	// 계층구조 충돌도 생각해야함 - 총알 피격판정용

	// class DLL PhysicsComponent : public IPhysicsComponent 
	// {
	// protected:
	// 	virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	// public:
	// 	virtual void Update(float timeElapsed) override;
	// };

	REGISTER_COMPONENT_ID(DynamicRigidComponent);
}