#pragma once
#include "IPhysicsComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include <functional>

namespace KG::Core
{
	class GameObject;
};

namespace physx
{
	class PxActor;
	class PxPhysics;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxRigidStatic;
	class PxFilterData;
	class PxErrorCallback;
}

namespace KG::Component
{
	class TransformComponent;

	enum class SHOW_COLLISION_BOX {
		NONE = 0, GRID, BOX
	};

	enum class FilterGroup {
		eFLOOR = (1 << 0),
		eBUILDING = (1 << 1),
		eBOX = (1 << 2),
		eENEMY = (1 << 3),
		ePLAYER = (1 << 4)
	};

	struct CollisionBox {
		DirectX::XMFLOAT3 center;
		DirectX::XMFLOAT3 scale;
		CollisionBox() {
			center = { 0,0,0 };
			scale = { 1,1,1 };
		}
	};

	class DLL IRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox											collisionBox;
		TransformComponent*										transform = nullptr;
		SHOW_COLLISION_BOX										show = SHOW_COLLISION_BOX::BOX;
		FilterGroup												filter = FilterGroup::eBOX;						// enum type prop
		std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)> callback = nullptr;								// 매개변수로 둘의 위치 / 타입이 들어가야 할듯					// 충돌 대상 타입, 충돌 대상 위치, 내 위치
		physx::PxFilterData*		filterData = nullptr;
		bool kinetic = false;														// prop
		bool dynamic = false;
		unsigned int id = 0;
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		virtual void SetupFiltering(uint32_t filterGroup, uint32_t filterMask);
	public:
		IRigidComponent() {};
		virtual void PostUpdate(float timeElapsed) override {};
		virtual void Update(float timeElapsed) override {};
		CollisionBox& GetCollisionBox() { return collisionBox; }
		virtual void SetCollisionCallback(std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)>&& collisionCallback) { this->callback = collisionCallback; };
		std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)> GetCollisionCallback() { return callback; }
		virtual physx::PxActor* GetActor() { return nullptr; };
		virtual void AddForce(DirectX::XMFLOAT3 dir, float distance = 1.0f) {};
		virtual void SetVelocity(DirectX::XMFLOAT3 dir, float distance = 1.0f) {};
		virtual void SetAngularVelocity(DirectX::XMFLOAT3 angle) {};
		virtual void SetRotation(DirectX::XMFLOAT4 quat) {};
		void SetId(unsigned int id) { this->id = id; }
		unsigned int GetId() const { return this->id; }
		physx::PxFilterData* GetFilterData() { return filterData; }


	protected:
		std::function<void()> updateLambda = nullptr;
	public:
		// 임시 업데이트 람다함수
		// 후에 AI 컴포넌트로 뺄것
		virtual void SetUpdateCallback(std::function<void()>&& lam) { this->updateLambda = lam; };
		virtual bool IsDynamic() const { return dynamic; }
	};

	class DLL DynamicRigidComponent : public IRigidComponent {
	protected:
		bool						apply = false;
		physx::PxRigidDynamic*		actor;

		virtual void OnCreate(KG::Core::GameObject* gameObject) override final;
		void SetupFiltering(unsigned int filterGroup, unsigned int filterMask);
	public:
		DynamicRigidComponent();
		virtual void PostUpdate(float timeElapsed) override final;
		virtual void Update(float timeElapsed) override final;
		void Move(DirectX::XMFLOAT3 direction, float speed);
		void SetActor(physx::PxRigidDynamic* actor);
		virtual physx::PxActor* GetActor() override final { return reinterpret_cast<physx::PxActor*>(actor); };
		virtual void AddForce(DirectX::XMFLOAT3 dir, float distance = 1.0f) override;
		virtual void SetVelocity(DirectX::XMFLOAT3 dir, float distance = 1.0f) override;
		virtual void SetAngularVelocity(DirectX::XMFLOAT3 angle) override;
		virtual void SetRotation(DirectX::XMFLOAT4 quat) override;
		// raycast 테스트



	private:
		// 참고할 코드 (저장 정보)
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		scaleProp;
		KG::Core::SerializableProperty<bool>					applyProp;
		KG::Core::SerializableEnumProperty<SHOW_COLLISION_BOX>	showProp;
		KG::Core::SerializableEnumProperty<FilterGroup>			filterProp;

	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};


	class DLL StaticRigidComponent : public IRigidComponent {
	protected:
		physx::PxRigidStatic*	actor;
		virtual void OnCreate(KG::Core::GameObject* gameObject) override final;
		void SetupFiltering(uint32_t filterGroup, uint32_t filterMask);
	public:
		StaticRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetActor(physx::PxRigidStatic* actor);
		virtual physx::PxActor* GetActor() override final { return reinterpret_cast<physx::PxActor*>(actor); };
	private:
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		scaleProp;
		KG::Core::SerializableEnumProperty<SHOW_COLLISION_BOX>	showProp;
		KG::Core::SerializableEnumProperty<FilterGroup>			filterProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	REGISTER_COMPONENT_ID(DynamicRigidComponent);
	REGISTER_COMPONENT_ID(StaticRigidComponent);
}