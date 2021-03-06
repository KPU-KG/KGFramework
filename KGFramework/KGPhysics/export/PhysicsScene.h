#pragma once

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif // PHYSICSPART_EXPORTS

#include <DirectXMath.h>
#include <vector>
#include <unordered_map>
#include "IPhysicsScene.h"
#include "PhysicsSystem.h"

namespace physx 
{
	class PxDefaultAllocator;
	class PxDefaultErrorCallback;
	class PxErrorCallback;
	class PxFoundation;
	class PxPhysics;
	class PxDefaultCpuDispatcher;
	class PxPvd;
	class PxScene;
	class PxRigidDynamic;
	class PxActor;
}

namespace KG::Component 
{
	class DynamicRigidComponent;
	class IRigidComponent;
}

namespace KG::Physics 
{
	enum PHYSICS_CALLBACK {
		CONSTRAINT, WAKE, SLEEP, TRIGGER, ADVANCE
	};

	class PhysicsEventCallback;
	// using namespace physx;
	class DLL PhysicsScene : public IPhysicsScene {
	private:
		static inline PhysicsScene* instance = nullptr;
	protected:
		bool initialized = false;
		physx::PxDefaultAllocator*		allocator;
		physx::PxErrorCallback*			errorCallback;
		physx::PxFoundation*			foundation;
		physx::PxPhysics*				physics;
		physx::PxDefaultCpuDispatcher*	cpuDispatcher;
		physx::PxPvd*					pvd;
		physx::PxScene*					scene;
		// physx::PxErrorCallback*			physicsErrorCallback;
		PhysicsEventCallback*			physicsEventCallback;

		float							accumulator = 0.0f;


		struct PhysicsSystems;
		PhysicsSystems* physicsSystems = nullptr;
		bool CreateScene(float gravity);

	public:
		PhysicsScene();
		virtual void Initialize() override;
		virtual bool Advance(float timeElapsed) override;


        virtual bool SetGUIContext(ImGuiContext* context) override
        {
            ImGui::SetCurrentContext(context);
            return true;
        }
		// 임시 지평면
		virtual void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid) override final;
		virtual void AddStaticActor(KG::Component::StaticRigidComponent* rigid) override final;
		virtual void AddFloor(float height) override final;
		virtual KG::Component::DynamicRigidComponent* GetNewDynamicRigidComponent() override final;
		virtual KG::Component::StaticRigidComponent* GetNewStaticRigidComponent() override final;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override final;
		virtual void ReleaseActor(KG::Component::IRigidComponent* comp) override final;
        virtual KG::Component::IRigidComponent* QueryRaycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId = 0, uint32_t mask = 0) override final;
        virtual RaycastResult QueryRaycastResult(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId = 0) override final;
		virtual bool IsInitialized() const override final;
		static PhysicsScene* GetInstance() { return instance; }

		virtual std::vector < std::pair<std::pair<int, int>, std::pair<int, int>>> GetStaticActorExtents() override final;
	};
}
