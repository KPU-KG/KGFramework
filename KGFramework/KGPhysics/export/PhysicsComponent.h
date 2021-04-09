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
	class PxRigidActor;
	class PxRigidDynamic;
	class PxRigidStatic;
	// PxU32;
	
	// class PxActor;
	// PxU32;
}
namespace KG::Component
{
	class TransformComponent;
	
	// 1. 컴포넌트 생성할 때 람다 함수로 넘겨줌
	// 2. 이벤트 콜백 클래스에서 람다 함수 가지기
	// 3. 이벤트에서 람다 함수 실행
	// unordered map으로 처리하면 될듯? (이벤트 종류, 함수) 이렇게??

	// class PhysicsEventCallback;

	void SetupFiltering(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);

	enum SHOW_COLLISION_BOX {
		NONE, GRID, BOX
	};


	struct CollisionBox {
		DirectX::XMFLOAT3 center;
		DirectX::XMFLOAT3 scale;
		CollisionBox() {
			center = { 0,0,0 };
			scale = { 1,1,1 };
		}
	};

	class DLL DynamicRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox				collisionBox;
		TransformComponent*			transform;		
		physx::PxRigidDynamic*		actor;			
		bool						apply = false;
		SHOW_COLLISION_BOX			show = BOX;

		// PhysicsEventCallback*		eventCallback;

		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		DynamicRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void Move(DirectX::XMFLOAT3 direction, float speed);
		void SetActor(physx::PxRigidDynamic* actor);
		// void OnContact
	private:
		// 참고할 코드 (저장 정보)
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		scaleProp;
		KG::Core::SerializableProperty<bool>					applyProp;
		KG::Core::SerializableEnumProperty<SHOW_COLLISION_BOX>	showProp;

	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};
	class DLL StaticRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox			collisionBox;
		TransformComponent*		transform;
		physx::PxRigidStatic*	actor;
		SHOW_COLLISION_BOX		show;
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
	public:
		StaticRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetActor(physx::PxRigidStatic* actor);
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		scaleProp;
		KG::Core::SerializableEnumProperty<SHOW_COLLISION_BOX>	showProp;
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
	REGISTER_COMPONENT_ID(StaticRigidComponent);
}