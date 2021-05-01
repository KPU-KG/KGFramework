#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "Protocol.h"
#include "Debug.h"
#include <functional>

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif


namespace KG::Server
{
	class Server;
};
namespace KG::Component
{
	class DLL SBaseComponent : public IComponent
	{
	protected:
		KG::Server::NET_OBJECT_ID networkObjectId = KG::Server::NULL_NET_OBJECT_ID;
		KG::Server::Server* server;
	public:
		void SetNetObjectId(KG::Server::NET_OBJECT_ID id);
		void SetServerInstance(KG::Server::Server* server);
		void BroadcastPacket(void* packet, KG::Server::SESSION_ID ignoreId = 0);
		void SendPacket(KG::Server::SESSION_ID sessionId, void* packet);
		bool ProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender);
	};


}