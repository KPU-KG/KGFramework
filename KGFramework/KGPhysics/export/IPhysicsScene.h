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
	class DLL IPhysicsScene {
	protected:
		PhysicsDesc desc;
		IPhysicsScene() = default;
		float							stepSize = 1.0f / 120.0f;
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
		// virtual void AddStaticActor(DirectX::XMFLOAT3 position, float width, float height, float depth) = 0;
		virtual void AddStaticActor(KG::Component::StaticRigidComponent* rigid) = 0;
		virtual void AddFloor(float height) = 0;

		virtual KG::Component::DynamicRigidComponent* GetNewDynamicRigidComponent() = 0;
		virtual KG::Component::StaticRigidComponent* GetNewStaticRigidComponent() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;
		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		}
		virtual KG::Component::IRigidComponent* QueryRaycast(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float maxDistance, unsigned int myId = 0) = 0;
		void SetStepSize(float stepSize) {
			this->stepSize = stepSize;
		}
	};

	DLL KG::Physics::IPhysicsScene* GetPhysicsScene();
}