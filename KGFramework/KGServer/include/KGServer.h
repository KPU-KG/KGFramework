#pragma once
#include "IKGServer.h"
#include <vector>
#include <stack>
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
#include "ServerEnemyUnitComponent.h"
#include "ServerLobbyComponent.h"
#include "ServerProjectileComponent.h"
#include "ServerCubeAreaRed.h"
#include "ServerSystems.h"

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
		NET_OBJECT_ID objectIdStart = LOBBY_ID + 1;

		EX_OVERLAPPED acceptOver;

		//Server System
		KG::System::SGameManagerComponentSystem sGameManagerSystem;
		KG::System::SPlayerComponentSystem sPlayerSystem;
		KG::System::SEnemyMechComponentSystem sEnemyMechSystem;
		KG::System::SEnemyCrawlerComponentSystem sEnemyCrawlerSystem;
		KG::System::SEnemyTurretComponentSystem sEnemyTurretSystem;
		KG::System::EnemyGeneratorSystem enemyGeneratorSystem;
		KG::System::SProjectileComponentSystem sProjectileSystem;
		KG::System::SCrawlerMissileComponentSystem sCrawlerMissileSystem;
		KG::System::SCubeAreaRedSystem sCubeAreaRedSystem;
		KG::System::SLobbyComponentSystem sLobbySystem;
		KG::Physics::IPhysicsScene* physicsScene;

		std::stack<KG::Server::NET_OBJECT_ID> disconnectedPlayerId;

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

		int playernum = -1;

	public:
		std::mutex worldLock;
		int currentnum = 0;
		

		NET_OBJECT_ID GetNewObjectId();
		void SetSessionState(SESSION_ID session, KG::Server::PLAYER_STATE state);
		void SetSessionId(SESSION_ID session, KG::Server::NET_OBJECT_ID id);
		void SetPlayerNum(int num);
		int GetPlayerNum();
		bool isPlay = false;
		//void AddPlayer();

		// IServer??(??) ???? ??????
		virtual void Initialize() override;
		virtual void Start(bool lock) override;
		virtual void Close() override;
		virtual void LockWorld() override;
		virtual void UnlockWorld() override;
		virtual KG::Component::SGameManagerComponent* GetNewGameManagerComponent() override;
		virtual KG::Component::SPlayerComponent* GetNewPlayerComponent() override;
		virtual KG::Component::SEnemyMechComponent* GetNewEnemyMechComponent() override;
		virtual KG::Component::SEnemyCrawlerComponent* GetNewEnemyCrawlerComponent() override;
		virtual KG::Component::SEnemyTurretComponent* GetNewEnemyTurretComponent() override;
		virtual KG::Component::EnemyGeneratorComponent* GetNewEnemyGeneratorComponent() override;
		virtual KG::Component::SProjectileComponent* GetNewProjectileComponent() override;
		virtual KG::Component::SLobbyComponent* GetNewLobbyComponent() override;
		virtual KG::Component::SCubeAreaRedComponent* GetNewCubeAreaRedComponent() override;
		virtual KG::Component::SCrawlerMissileComponent* GetNewCrawlerMissileComponent() override;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;
		virtual void DrawImGUI() override;
		virtual bool isStarted() const override;
		virtual void SetPhysicsScene(KG::Physics::IPhysicsScene* physicsScene) override;
		virtual KG::Physics::IPhysicsScene* GetPhysicsScene() override;
		virtual void SetServerObject(KG::Server::NET_OBJECT_ID id, KG::Component::SBaseComponent* obj);
		virtual void BroadcastPacket(void* packet, SESSION_ID ignore = SERVER_ID);
		virtual void SendPacket(SESSION_ID playerId, void* packet);
		virtual KG::Component::SBaseComponent* FindNetObject(NET_OBJECT_ID id);

		std::stack<KG::Server::NET_OBJECT_ID>& GetDisconnectedPlayerId();

		// IServer??(??) ???? ??????
		virtual void Update(float elapsedTime) override;
	};
};