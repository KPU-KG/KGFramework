#pragma once
#include "ISystem.h"
#include "PhysicsComponent.h"
namespace KG::System
{
	using namespace KG::Component;
	struct PhysicsSystem : public IComponentSystem<DynamicRigidComponent>
	{
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}