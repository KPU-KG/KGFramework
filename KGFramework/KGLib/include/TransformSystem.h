#pragma once
#include "ISystem.h"
#include "Transform.h"
namespace KG::System
{
	class TransformSystem : public IComponentSystem<TransformComponent>
	{
	public:
		TransformSystem() = default;
		void Initialize();
		virtual void OnGetNewComponent( TransformComponent* tran ) override;
		virtual void OnUpdate(float elapsedTime) override;
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
};