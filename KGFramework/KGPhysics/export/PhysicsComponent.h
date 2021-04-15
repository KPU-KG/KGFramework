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
}

namespace KG::Component
{
	class TransformComponent;

	void SetupFiltering(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);

	enum SHOW_COLLISION_BOX {
		NONE, GRID, BOX
	};

	enum FilterGroup {
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

	// conponent�� �ݹ� �Լ� �߰� (�浹�� ��� Ÿ��, �浹 ��ġ�� �Ű������� ����)
	// scene�� filter �Լ����� �Ű����� ����
	struct CollisionInfo {
		unsigned int otherType;
		DirectX::XMFLOAT3 otherPosition;
		DirectX::XMFLOAT3 myPosition;
	};

	using  CollisionCallback = std::function<CollisionInfo(unsigned int, DirectX::XMFLOAT3, DirectX::XMFLOAT3)>;

	class DLL DynamicRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox				collisionBox;
		TransformComponent*			transform;		
		physx::PxRigidDynamic*		actor;			
		bool						apply = false;
		SHOW_COLLISION_BOX			show = SHOW_COLLISION_BOX::BOX;

		// �߰������ �Ұ� (04.11 ����)
		// 1. �������� ���
		// 2. �浹 ����
		// 3. �ݹ��Լ� ��� - �̰Ŵ� ��ũ��Ʈ�� �ؾ߰���??
		// 4. KINETIC ��� �߰�
		FilterGroup filter = FilterGroup::eBOX;															// enum type prop
		CollisionCallback callback = nullptr;					// �Ű������� ���� ��ġ / Ÿ���� ���� �ҵ�
																										// �浹 ��� Ÿ��, �浹 ��� ��ġ, �� ��ġ
		bool kinetic;																					// prop

		virtual void OnCreate(KG::Core::GameObject* gameObject) override;
		void SetupFiltering(uint32_t filterGroup, uint32_t filterMask);
	public:
		DynamicRigidComponent();
		virtual void PostUpdate(float timeElapsed) override;
		virtual void Update(float timeElapsed) override;
		CollisionBox& GetCollisionBox() { return collisionBox; }
		void Move(DirectX::XMFLOAT3 direction, float speed);
		void SetActor(physx::PxRigidDynamic* actor);

		void SetupCollisionCallback(CollisionCallback&& collisionCallback);
		CollisionCallback GetCollisionCallback() { return callback; }
	private:
		// ������ �ڵ� (���� ����)
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


	class DLL StaticRigidComponent : public IPhysicsComponent {
	protected:
		CollisionBox			collisionBox;
		TransformComponent*		transform;
		physx::PxRigidStatic*	actor;
		SHOW_COLLISION_BOX		show = SHOW_COLLISION_BOX::BOX;
		FilterGroup				filter = FilterGroup::eBUILDING;
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
		KG::Core::SerializableEnumProperty<FilterGroup>			filterProp;
	public:
		virtual void OnDataLoad(tinyxml2::XMLElement* componentElement);
		virtual void OnDataSave(tinyxml2::XMLElement* parentElement);
		virtual bool OnDrawGUI();
	};

	REGISTER_COMPONENT_ID(DynamicRigidComponent);
	REGISTER_COMPONENT_ID(StaticRigidComponent);
}