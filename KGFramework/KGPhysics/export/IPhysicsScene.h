#pragma once

#include "ISystem.h"
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
    /* ±èÅÂÇü Ãß°¡ */
    struct DLL RaycastResult
    {
        KG::Component::IRigidComponent* targetRigid = nullptr;
        DirectX::XMFLOAT3 hitPosition = DirectX::XMFLOAT3(0, 0, 0);
        DirectX::XMFLOAT3 normal = DirectX::XMFLOAT3(0, 0, 0);
        DirectX::XMFLOAT2 uv = DirectX::XMFLOAT2(0, 0);
        float distance = 0;
    };


	class DLL IPhysicsScene {
	protected:
		PhysicsDesc desc;
		IPhysicsScene() = default;
		float							stepSize = 1.0f / 60.0f;
	public:
		void SetDsec(const PhysicsDesc& desc);

		virtual void Initialize(const PhysicsDesc& desc)
		{
			this->SetDsec(desc);
			this->Initialize();
		}

		virtual void Initialize() = 0;
		virtual bool Advance(float timeElapsed) = 0;

		virtual void AddDynamicActor(KG::Component::DynamicRigidComponent* rigid) = 0;
		virtual void AddStaticActor(KG::Component::StaticRigidComponent* rigid) = 0;
		virtual void AddFloor(float height) = 0;

		virtual KG::Component::DynamicRigidComponent* GetNewDynamicRigidComponent() = 0;
		virtual KG::Component::StaticRigidComponent* GetNewStaticRigidComponent() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;
		virtual void ReleaseActor(KG::Component::IRigidComponent* comp) = 0;
		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		}
		virtual KG::Component::IRigidComponent* QueryRaycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId = 0) = 0;
        virtual RaycastResult QueryRaycastResult(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId = 0) = 0;
		void SetStepSize(float stepSize) {
			this->stepSize = stepSize;
		}
		virtual std::vector < std::pair<std::pair<int, int>, std::pair<int, int>>> GetStaticActorExtents() = 0;
		virtual bool IsInitialized() const = 0;

	};

	DLL KG::Physics::IPhysicsScene* GetPhysicsScene();
}