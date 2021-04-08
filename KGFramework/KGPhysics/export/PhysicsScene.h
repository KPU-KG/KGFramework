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
	enum PHYSICS_CALLBACK {
		CONSTRAINT, WAKE, SLEEP, TRIGGER, ADVANCE
	};

	struct FilterGroup {
		enum Enum {
			eFLOOR		= (1 << 0),
			eBUILDING	= (1 << 1),
			eBOX		= (1 << 2)
		};
	};

	class PhysicsEventCallback;
	class PhysicsCollisionCallback;
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
		PhysicsEventCallback*			physicsEventCallback;

		float							accumulator = 0.0f;
		float							stepSize = 1.0f / 60.0f;

		struct PhysicsSystems;
		PhysicsSystems* physicsSystems = nullptr;
		bool CreateScene(float gravity);
	public:
		PhysicsScene();
		virtual void Initialize() override;
		virtual bool Advance(float timeElapsed) override;

		// 임시 지평면
		virtual void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid) override;
		virtual void AddStaticActor(KG::Component::StaticRigidComponent* rigid) override;
		virtual void AddFloor(float height) override;
		virtual KG::Component::DynamicRigidComponent* GetNewDynamicRigidComponent() override;
		virtual KG::Component::StaticRigidComponent* GetNewStaticRigidComponent() override;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

		static PhysicsScene* GetInstance() { return instance; }
	};
}
