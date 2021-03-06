#pragma once
#include "IPhysicsComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "MathHelper.h"
#include <functional>

namespace KG::Core
{
	class GameObject;
};
namespace KG::Physics
{
    class IPhysicsScene;
}

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
	class IRigidComponent;
	using CollisionCallbackFunc = std::function<void(KG::Component::IRigidComponent*, KG::Component::IRigidComponent*)>;

	enum class RaycastType {
		BULLET_HIT = 0
	};

	using RaycastCallbackFunc = std::function<void(KG::Component::RaycastType, KG::Component::IRigidComponent*)>;

	enum class COLLISION_SHAPE {
		NONE = 0, 
		BOX
	};

	enum class FilterGroup {
		eNONE = 0,
		eFLOOR = (1 << 0),
		eBUILDING = (1 << 1),
		eBOX = (1 << 2),
		eENEMY = (1 << 3),
		ePLAYER = (1 << 4),
		eBULLET = (1 << 5)
	};

	struct CollisionBox {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT3 rotation;
		CollisionBox() {
			position = { 0,0,0 };
			rotation = { 0,0,0 };
			scale = { 1,1,1 };
		}
        DirectX::XMFLOAT4X4 GetMatrix() const;
	};

	class DLL IRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox											collisionBox;
		TransformComponent*										transform = nullptr;
		COLLISION_SHAPE											show = COLLISION_SHAPE::BOX;
		FilterGroup												filter = FilterGroup::eBOX;
		FilterGroup												mask = FilterGroup::eNONE;

		uint32_t												filterGroup = 0;
		uint32_t												maskGroup = 0;
		// uint32_t												queryMaskGroup;

		KG::Component::CollisionCallbackFunc					collisionCallback = nullptr;
		RaycastCallbackFunc										raycastCallback = nullptr;
		physx::PxFilterData*									filterData = nullptr;
        KG::Physics::IPhysicsScene* scene = nullptr;
		bool kinetic = false;														
		bool dynamic = false;
		unsigned int id = 0;
		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		virtual void SetupFiltering(uint32_t filterGroup, uint32_t filterMask);
	public:
		IRigidComponent();
		virtual void PostUpdate(float timeElapsed) override {};
		virtual void Update(float timeElapsed) override {};
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void SetCollisionBox(CollisionBox& box);

		virtual void SetCollisionCallback(KG::Component::CollisionCallbackFunc&& collisionCallback);;
		KG::Component::CollisionCallbackFunc GetCollisionCallback();

		virtual void SetRaycastCallback(KG::Component::RaycastCallbackFunc& raycastCallback);
		virtual KG::Component::RaycastCallbackFunc GetRaycastCallback() const;

        virtual KG::Physics::IPhysicsScene* GetScene() const { return this->scene; }
		virtual physx::PxActor* GetActor() { return nullptr; };
		virtual void AddForce(DirectX::XMFLOAT3 dir, float distance = 1.0f) {};
		virtual void SetVelocity(DirectX::XMFLOAT3 dir, float distance = 1.0f) {};
		virtual void AddTorque(DirectX::XMFLOAT3 axis, float power) {};
		virtual void SetRotation(DirectX::XMFLOAT4 quat) {};
		void SetId(unsigned int id) { this->id = id; }
		unsigned int GetId() const { return this->id; }
		physx::PxFilterData* GetFilterData() { return filterData; }
		uint32_t GetFilterGroup() { return filterGroup; }
		uint32_t GetFilterMask() { return maskGroup; }
		virtual void SetPosition(DirectX::XMFLOAT3 pos) {};
		virtual void SetApply(bool apply) {};
		virtual DirectX::XMFLOAT3 GetVelocity() const { return DirectX::XMFLOAT3(); };
		virtual void ReleaseActor() = 0;
		void AddFilterGroup(FilterGroup filterGroup, FilterGroup filterMask);
		// void SetQueryMaskGroup(uint32_t mask);

	public:
		// property
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		positionProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		scaleProp;
		KG::Core::SerializableProperty<DirectX::XMFLOAT3>		rotationProp;
		KG::Core::SerializableEnumProperty<FilterGroup>			filterProp;
		KG::Core::SerializableEnumProperty<COLLISION_SHAPE>		shapeProp;


	protected:
		// std::function<void()> updateLambda = nullptr;
	public:
		// ???? ???????? ????????
		// ???? AI ?????????? ????
		// virtual void SetUpdateCallback(std::function<void()>&& lam) { this->updateLambda = lam; };
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
		virtual void AddTorque(DirectX::XMFLOAT3 axis, float power) override;
		virtual void SetRotation(DirectX::XMFLOAT4 quat) override;
		virtual void SetPosition(DirectX::XMFLOAT3 pos) override;
		virtual DirectX::XMFLOAT3 GetVelocity() const override;
		virtual void SetApply(bool apply) override { this->apply = apply; }
		virtual void ReleaseActor() override;
		void SetLinearLock(bool x, bool y, bool z);
		void SetAngularLock(bool x, bool y, bool z);
		void SetRigidFlags(bool ax, bool ay, bool az, bool lx, bool ly, bool lz);
		// raycast ??????

	private:
		// ?????? ???? (???? ????)
		KG::Core::SerializableProperty<bool>					applyProp;
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
		virtual void ReleaseActor() override;

	private:
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	REGISTER_COMPONENT_ID(DynamicRigidComponent);
	REGISTER_COMPONENT_ID(StaticRigidComponent);
}