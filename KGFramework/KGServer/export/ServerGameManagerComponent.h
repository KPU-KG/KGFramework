#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"
#include <functional>
#include <concurrent_unordered_map.h>
namespace KG::Component
{
	class SPlayerComponent;
	class DLL SGameManagerComponent : public SBaseComponent
	{
	public:
		float updatetimer = 0;
		concurrency::concurrent_unordered_map<KG::Server::NET_OBJECT_ID, KG::Component::SPlayerComponent*> playerObjects;
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
	};
	REGISTER_COMPONENT_ID(SGameManagerComponent);

	

}