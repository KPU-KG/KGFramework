#pragma once
#include "IKGServer.h"
#include <vector>
#include <ppl.h>
#include <concurrent_unordered_map.h>

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>
#include <thread>
#include <atomic>
#include <MSWSock.h>

#include "Session.h"

#include "ServerGameManagerComponent.h"
#include "ServerPlayerControllerComponent.h"

namespace KG::Server
{
	class Server : public IServer
	{
		HANDLE hIocp;
		SYSTEM_INFO systemInfo;
		SOCKET listenSocket;


		std::vector<std::thread> iocpWorkers;

		// https://docs.microsoft.com/ko-kr/cpp/parallel/concrt/parallel-containers-and-objects?view=msvc-160#unordered_map
		concurrency::concurrent_unordered_map<SESSION_ID, SESSION> players;
		concurrency::concurrent_unordered_map<NET_OBJECT_ID, KG::Component::SBaseComponent*> netObjects;

		std::mutex sessionIdStartMutex;
		SESSION_ID sessionIdStart = SERVER_ID + 1;

		std::mutex objectIdStartMutex;
		NET_OBJECT_ID objectIdStart = SCENE_CONTROLLER_ID + 1;

		EX_OVERLAPPED acceptOver;

		//Server System
		KG::Component::SGameManagerComponentSystem sGameManagerSystem;
		KG::Component::SPlayerComponentSystem sPlayerSystem;


		static void IOCPWorker(Server* server);

		//Worker Thread
		SESSION_ID GetNewPlayerId();
		void SendLoginOkPacket(SESSION_ID playerId);
		void SendWorldState(SESSION_ID playerId);
		void SendRemovePlayer(SESSION_ID playerId, SESSION_ID targetId);
		void SendAddPlayer(SESSION_ID playerId, SESSION_ID targetId);
		void Disconnect(SESSION_ID playerId);
		void DoRecv(SESSION_ID key);
		void ProcessPacket(SESSION_ID playerId, unsigned char* buffer);

	public:
		std::mutex worldLock;
		NET_OBJECT_ID GetNewObjectId();

		// IServer을(를) 통해 상속됨
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void Close() override;
		virtual void LockWorld() override;
		virtual void UnlockWorld() override;
		virtual KG::Component::SGameManagerComponent* GetNewGameManagerComponent() override;
		virtual KG::Component::SPlayerComponent* GetNewPlayerComponent() override;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;
		virtual void DrawImGUI() override;
		virtual bool isStarted() const override;
		virtual void SetServerObject(KG::Server::NET_OBJECT_ID id, KG::Component::SBaseComponent* obj);
		virtual void BroadcastPacket(void* packet, SESSION_ID ignore = SERVER_ID);
		virtual void SendPacket(SESSION_ID playerId, void* packet);

		// IServer을(를) 통해 상속됨
		virtual void Update(float elapsedTime) override;
	};
};