#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include <functional>
namespace KG::Component
{
	class PlayerControllerComponent : public IComponent
	{
	private:
	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnDrawGUI();
	};
	REGISTER_COMPONENT_ID(PlayerControllerComponent);

	class PlayerControllerComponentSystem : public KG::System::IComponentSystem<PlayerControllerComponent>
	{
	public:
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem��(��) ���� ��ӵ�
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;

	};
}