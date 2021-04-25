#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>

namespace KG::Component
{
	class DLL SPlayerComponent : public SBaseComponent
	{
	public:
		float updatetimer = 0;
		KG::Server::NET_OBJECT_ID playerObjectIds[4] = { 0, };
		KG::Packet::RawFloat3 positions[4];
		KG::Packet::CS_INPUT inputs[4];

		void InitData(KG::Server::NET_OBJECT_ID[], KG::Packet::RawFloat3[]);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
	};
	REGISTER_COMPONENT_ID(SPlayerComponent);


	class DLL SPlayerComponentSystem : public KG::Component::SBaseComponentSystem<SPlayerComponent>
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