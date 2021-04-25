#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include <functional>

namespace KG::Component
{
	class DLL CGameManagerComponent : public CBaseComponent
	{
	public:
		CBaseComponent* playerController;
		TransformComponent* playerTransform;
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
	REGISTER_COMPONENT_ID(CGameManagerComponent);


	class DLL CGameManagerComponentSystem : public KG::Component::CBaseComponentSystem<CGameManagerComponent>
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