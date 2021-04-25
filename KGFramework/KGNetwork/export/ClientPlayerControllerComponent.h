#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include "InputManager.h"
#include <functional>

namespace KG::Component
{
	class TransformComponent;
	class DLL CPlayerControllerComponent : public CBaseComponent
	{
	public:
		KG::Input::InputManager* input;
		float updatetimer = 0;
		int id = -1;
		KG::Component::TransformComponent* transform = nullptr;
		KG::Packet::CS_INPUT inputCache;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();

		// CBaseComponent��(��) ���� ��ӵ�
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
	};
	REGISTER_COMPONENT_ID(CPlayerControllerComponent);


	class DLL CPlayerControllerComponentSystem : public KG::Component::CBaseComponentSystem<CPlayerControllerComponent>
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