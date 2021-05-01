#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Debug.h"
#include "Protocol.h"
#include <functional>

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


namespace KG::Server
{
	class Network;
};
namespace KG::Component
{
	class DLL CBaseComponent : public IComponent
	{
	protected:
		KG::Server::NET_OBJECT_ID networkObjectId = KG::Server::NULL_NET_OBJECT_ID;
		KG::Server::Network* network;
	public:
		void SetNetObjectId(KG::Server::NET_OBJECT_ID id);
		void SetNetworkInstance(KG::Server::Network* network);
		void SendPacket(void* packet);
		bool ProcessPacket(unsigned char* packet, KG::Packet::PacketType type);
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type);
	};

	//template<typename Ty>
	//class DLL CBaseComponentSystem : public KG::System::IComponentSystem<Ty>
	//{
	//protected:
	//	KG::Server::Network* network;
	//public:
	//	void SetNetworkInstance(KG::Server::Network* network)
	//	{
	//		this->network = network;
	//	}
	//	virtual void OnPostProvider(KG::Component::ComponentProvider& provider) override
	//	{
	//		provider.PostInjectionFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()),
	//			[this](KG::Core::GameObject* object)
	//			{
	//				auto* comp = this->GetNewComponent();
	//				comp->SetNetworkInstance(this->network);
	//				object->AddComponent<Ty>(comp);
	//				return comp;
	//			}
	//		);
	//		provider.PostGetterFunction(KG::Utill::HashString(KG::Component::ComponentID<Ty>::name()),
	//			[this]()->KG::Component::IComponent*
	//			{
	//				auto* comp = this->GetNewComponent();
	//				comp->SetNetworkInstance(this->network);
	//				return static_cast<KG::Component::IComponent*>(comp);
	//			}
	//		);

	//	}
	//};

}