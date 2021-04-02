#pragma once
#include "ISystem.h"
#include "PhysicsComponent.h"
namespace KG::System
{
	using namespace KG::Component;
	struct DynamicRigidSystem : public IComponentSystem<DynamicRigidComponent>
	{
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};

	struct StaticRigidSystem : public IComponentSystem<StaticRigidComponent>
	{
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}