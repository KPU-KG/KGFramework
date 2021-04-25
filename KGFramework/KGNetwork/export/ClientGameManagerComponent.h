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
		KG::Packet::CS_INPUT inputPacket = {};
		float updatetimer = 0;

		//KG::Component::CPlayerControllerComponent* teamControllers[4];
		//KG::Component::CPlayerControllerComponent* playerController;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();

		// CBaseComponent을(를) 통해 상속됨
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
	};
	REGISTER_COMPONENT_ID(CGameManagerComponent);


	class DLL CGameManagerComponentSystem : public KG::System::IComponentSystem<CGameManagerComponent>
	{
		KG::Server::Network* network = nullptr;
		virtual void OnGetNewComponent(CGameManagerComponent* comp)
		{
			comp->SetNetworkInstance(network);
		}
	public:
		void SetNetworkInstance(KG::Server::Network* network)
		{
			this->network = network;
		}
		virtual void OnUpdate(float elapsedTime) override
		{
			for ( auto& com : *this )
			{
				com.Update(elapsedTime);
			}
		}

		// IComponentSystem을(를) 통해 상속됨
		virtual void OnPostUpdate(float elapsedTime) override;
		virtual void OnPreRender() override;
	};
}