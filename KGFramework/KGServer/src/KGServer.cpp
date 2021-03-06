#include "pch.h"
#include "Protocol.h"
#include "ServerUtill.h"
#include "KGServer.h"
#include "ServerEnemyMechComponent.h"
#include "ServerGameManagerComponent.h"
#include <sstream>


DLL KG::Server::IServer* KG::Server::GetServer()
{
	return new Server();
}


void KG::Server::Server::IOCPWorker(Server* server)
{
	std::stringstream ss;
	ss << "IOCP Worker Started ThreadId : " << std::this_thread::get_id() << "\n";
	std::cout << ss.str();
	while ( true )
	{
		DWORD numBytes;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;
		//std::cout << "GQCS : Queued\n";
		BOOL ret = GetQueuedCompletionStatus(server->hIocp, &numBytes, &ikey, &over, INFINITE);
		SESSION_ID key = static_cast<SESSION_ID>(ikey);
		if ( FALSE == ret )
		{
			if ( SERVER_ID == key )
			{
				display_error("GQCS : ", WSAGetLastError());
				exit(-1);
			}
			else //에러 or 강제 종료
			{
				display_error("GQCS : ", WSAGetLastError());
				server->Disconnect(key);
				continue;
			}
		}

		if ( numBytes == 0 && key != 0 ) // 정상 종료
		{
			server->Disconnect(key);
		}

		EX_OVERLAPPED* ex_over = reinterpret_cast<EX_OVERLAPPED*>(over);
		switch ( ex_over->op )
		{
			case OP_RECV:
			{
				auto& session = server->players[key];
				unsigned char* packet_ptr = ex_over->packetBuffer;
				int num_data = numBytes + session.prevSize;
				int packetSize = packet_ptr[0];

				while ( num_data >= packetSize ) // 다받음
				{
					server->ProcessPacket(key, packet_ptr);
					num_data -= packetSize;
					packet_ptr += packetSize;
					if ( 0 >= num_data ) break;
					packetSize = packet_ptr[0];
				}
				session.prevSize = num_data;
				if ( num_data != 0 )
				{
					memcpy(ex_over->packetBuffer, packet_ptr, num_data);
				}
				server->DoRecv(key);
			}
			break;
			case OP_SEND:
			{
				delete ex_over;
			}
			break;
			case OP_ACCEPT:
			{
				SESSION_ID clientId = server->GetNewPlayerId();
				if ( clientId > 0 )
				{
					std::cout << "Accept New Client ID : " << clientId << "\n";
					SESSION newSession;
					newSession.id = clientId;
					newSession.prevSize = 0;
					newSession.recvExOver.op = OP_RECV;
					newSession.socket = ex_over->csocket;
					newSession.state = PLAYER_STATE_CONNECTED;
					server->players.insert(
						std::make_pair(std::move(clientId), std::move(newSession))
					);
					DWORD flags = 0;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(ex_over->csocket), server->hIocp, clientId, NULL); //소켓 등록
					server->DoRecv(clientId);
				}
				else
				{
					closesocket(ex_over->csocket);
				}

				ex_over->op = OP_ACCEPT;
				memset(&ex_over->over, 0, sizeof(ex_over->over));
				ex_over->csocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				AcceptEx(server->listenSocket, ex_over->csocket, ex_over->packetBuffer, 0, 32, 32, 0, &ex_over->over);
			}
			break;
		}
	}
}

void KG::Server::Server::Initialize()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	std::wcout.imbue(std::locale("korean"));
	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	assert(hIocp != 0 && "hIocp is 0");

	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), hIocp, SERVER_ID, 0);

	int option = TRUE;    //TRUE : Nagle Off, FALSE : Nagle On
	setsockopt(listenSocket,        //소켓    
		IPPROTO_TCP,    //소켓 레벨
		TCP_NODELAY,    //소켓 옵션
		(const char*)&option, //옵션값의 주소값
		sizeof(option));      //옵션값의 

	//Init Systems
	this->sGameManagerSystem.SetServerInstance(this);
	this->sLobbySystem.SetServerInstance(this);
}

void KG::Server::Server::Start(bool lock)
{
	std::cout << "Start Server Process\n";
	if(lock) this->LockWorld();
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	memset(&acceptOver.over, 0, sizeof(acceptOver.over));
	acceptOver.op = OP_ACCEPT;
	acceptOver.csocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	// int option = TRUE;    //TRUE : Nagle Off, FALSE : Nagle On
	// setsockopt(acceptOver.csocket,        //소켓    
	// 	IPPROTO_TCP,    //소켓 레벨
	// 	TCP_NODELAY,    //소켓 옵션
	// 	(const char*)&option, //옵션값의 주소값
	// 	sizeof(option));      //옵션값의 

	int ret = AcceptEx(listenSocket, acceptOver.csocket, acceptOver.packetBuffer, 0, 32, 32, 0, &acceptOver.over);
	if ( ret == false )
	{
		int error = WSAGetLastError();
		if ( error != WSA_IO_PENDING )
		{
			display_error("AcceptEx", error);
		}
	}

	//Query Core
	GetSystemInfo(&systemInfo);
	this->iocpWorkers.reserve(systemInfo.dwNumberOfProcessors);
	systemInfo.dwNumberOfProcessors = 1;
	for ( size_t i = 0; i < systemInfo.dwNumberOfProcessors; i++ )
	{
		this->iocpWorkers.emplace_back(Server::IOCPWorker, this);
	}
}

void KG::Server::Server::Close()
{
	for ( auto& i : this->iocpWorkers )
	{
		i.join();
	}
	closesocket(listenSocket);
	WSACleanup();
}

void KG::Server::Server::LockWorld()
{
	worldLock.lock();
	std::stringstream ss;
	//ss << "World Lock / ThreadId : " << std::this_thread::get_id() << "\n";
	std::cout << ss.str();
}

void KG::Server::Server::UnlockWorld()
{
	std::stringstream ss;
	//ss << "World UnLock / ThreadId : " << std::this_thread::get_id() << "\n";
	std::cout << ss.str();
	worldLock.unlock();
}

KG::Component::SGameManagerComponent* KG::Server::Server::GetNewGameManagerComponent()
{
	auto* comp = this->sGameManagerSystem.GetNewComponent();
	comp->SetServerInstance(this);
	comp->SetPhysicsScene(this->physicsScene);
	return comp;
}

KG::Component::SPlayerComponent* KG::Server::Server::GetNewPlayerComponent()
{
	auto* comp = this->sPlayerSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SEnemyMechComponent* KG::Server::Server::GetNewEnemyMechComponent()
{
	auto* comp = this->sEnemyMechSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SEnemyCrawlerComponent* KG::Server::Server::GetNewEnemyCrawlerComponent()
{
	auto* comp = this->sEnemyCrawlerSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SEnemyTurretComponent* KG::Server::Server::GetNewEnemyTurretComponent()
{
	auto* comp = this->sEnemyTurretSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::EnemyGeneratorComponent* KG::Server::Server::GetNewEnemyGeneratorComponent()
{
	auto* comp = this->enemyGeneratorSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SProjectileComponent* KG::Server::Server::GetNewProjectileComponent()
{
	auto* comp = this->sProjectileSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SLobbyComponent* KG::Server::Server::GetNewLobbyComponent()
{
	auto* comp = this->sLobbySystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SCubeAreaRedComponent* KG::Server::Server::GetNewCubeAreaRedComponent()
{
	auto* comp = this->sCubeAreaRedSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

KG::Component::SCrawlerMissileComponent* KG::Server::Server::GetNewCrawlerMissileComponent()
{
	auto* comp = this->sCrawlerMissileSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

void KG::Server::Server::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
	this->sGameManagerSystem.OnPostProvider(provider);
	this->sPlayerSystem.OnPostProvider(provider);
	this->sEnemyMechSystem.OnPostProvider(provider);
	this->sEnemyCrawlerSystem.OnPostProvider(provider);
	this->sEnemyTurretSystem.OnPostProvider(provider);
	this->enemyGeneratorSystem.OnPostProvider(provider);
	this->sProjectileSystem.OnPostProvider(provider);
	this->sLobbySystem.OnPostProvider(provider);
	this->sCrawlerMissileSystem.OnPostProvider(provider);
	this->sCubeAreaRedSystem.OnPostProvider(provider);
}

void KG::Server::Server::DrawImGUI()
{
    ImGui::BulletText("Is Processing Server");
}

bool KG::Server::Server::isStarted() const
{
	return this->iocpWorkers.size() != 0;
}

void KG::Server::Server::SetPhysicsScene(KG::Physics::IPhysicsScene* physicsScene)
{
	this->physicsScene = physicsScene;
}

KG::Physics::IPhysicsScene* KG::Server::Server::GetPhysicsScene()
{
	return this->physicsScene;
}

void KG::Server::Server::SetServerObject(KG::Server::NET_OBJECT_ID id, KG::Component::SBaseComponent* obj)
{
	this->netObjects.insert(std::make_pair(id, obj));
}

//Worker 
KG::Server::SESSION_ID KG::Server::Server::GetNewPlayerId()
{
	std::lock_guard lg{ this->sessionIdStartMutex };
	int ret = this->sessionIdStart;
	this->sessionIdStart += 1;
	return ret;
}

void KG::Server::Server::BroadcastPacket(void* packet, SESSION_ID ignore)
{
	//std::cout << "BroadCast Packet!\n";
	for ( auto& i : this->players )
	{
		if ( i.first != ignore )
		{
			// std::shared_lock<std::shared_mutex> lg{ i.second.sessionLock };
			if ( i.second.state == PLAYER_STATE_INGAME )
			{
				this->SendPacket(i.first, packet);
			}
		}
	}
}

void KG::Server::Server::SendPacket(SESSION_ID playerId, void* packet)
{
	int packetSize = reinterpret_cast<unsigned char*>(packet)[0];
	int packetType = reinterpret_cast<unsigned char*>(packet)[1];

	//std::cout << "To client [" << playerId << "] : ";
	//std::cout << "Packet / Type : " << packetType << " / Size : " << packetSize << "\n";

	auto* exOver = new EX_OVERLAPPED();

	exOver->op = OP_SEND;
	memset(&exOver->over, 0, sizeof(WSAOVERLAPPED));
	memcpy(exOver->packetBuffer, packet, packetSize);
	exOver->wsaBuf[0].buf = (char*)exOver->packetBuffer;
	exOver->wsaBuf[0].len = packetSize;

	int ret = WSASend(players[playerId].socket, exOver->wsaBuf, 1, NULL, 0, &exOver->over, NULL);
	if ( ret != 0 )
	{
		int err_no = WSAGetLastError();
		if ( WSA_IO_PENDING != err_no )
		{
			display_error("Error in SendPacket : ", err_no);
		}
	}
}

KG::Component::SBaseComponent* KG::Server::Server::FindNetObject(NET_OBJECT_ID id)
{
	auto it = this->netObjects.find(id);
	if (it == this->netObjects.end())
		return nullptr;
	else
		return it->second;
}


std::stack<KG::Server::NET_OBJECT_ID>& KG::Server::Server::GetDisconnectedPlayerId()
{
	return this->disconnectedPlayerId;
	// TODO: 여기에 return 문을 삽입합니다.
}

void KG::Server::Server::Update(float elapsedTime)
{
	this->sGameManagerSystem.OnUpdate(elapsedTime);
	this->sPlayerSystem.OnUpdate(elapsedTime);
	this->sEnemyMechSystem.OnUpdate(elapsedTime);
	this->sEnemyCrawlerSystem.OnUpdate(elapsedTime);
	this->sEnemyTurretSystem.OnUpdate(elapsedTime);
	this->enemyGeneratorSystem.OnUpdate(elapsedTime);
	this->sProjectileSystem.OnUpdate(elapsedTime);
	this->sCrawlerMissileSystem.OnUpdate(elapsedTime);
	this->sCubeAreaRedSystem.OnUpdate(elapsedTime);
}

void KG::Server::Server::SendLoginOkPacket(SESSION_ID playerId)
{
}

void KG::Server::Server::SendWorldState(SESSION_ID playerId)
{
}

void KG::Server::Server::SendRemovePlayer(SESSION_ID playerId, SESSION_ID targetId)
{
	Packet::SC_REMOVE_PLAYER packet = {};
	auto it = this->netObjects.find(playerId);
	if (it != this->netObjects.end())
		it->second->SendPacket(targetId, (void*)&packet);

	//packet_s2c_remove_player p;
	//p.id = playerId;
	//p.type = SC_REMOVE_PLAYER;
	//p.size = sizeof(p);
	//SendPacket(targetId, &p);
}

void KG::Server::Server::SendAddPlayer(SESSION_ID playerId, SESSION_ID targetId)
{
	//packet_s2c_add_player p;
	//p.id = playerId;
	//p.type = SC_ADD_PLAYER;
	//p.size = sizeof(p);
	//p.x = players[playerId].x;
	//p.y = players[playerId].y;
	//p.race = 0;
	//SendPacket(targetId, &p);
}

void KG::Server::Server::Disconnect(SESSION_ID playerId)
{
	int playerCompId;
	std::cout << "Disconnected Client[" << playerId << "]\n";
	{
		std::lock_guard<std::shared_mutex> lg{ players[playerId].sessionLock };
		playerCompId = players[playerId].netId;

		auto it = this->netObjects.find(playerCompId);
		if (it != this->netObjects.end())
			it->second->Destroy();

		players[playerId].state = PLAYER_STATE_FREE;
		closesocket(players[playerId].socket);
		disconnectedPlayerId.push(playerCompId);
	}

	for ( auto& [id , pl] : players )
	{
		std::shared_lock<std::shared_mutex> lg2{ pl.sessionLock };
		if ( pl.state == PLAYER_STATE_INGAME )
		{
			SendRemovePlayer(playerCompId, pl.id);
		}
	}
	sLobbySystem.DisconnectPlayer(playerId);
}

void KG::Server::Server::DoRecv(SESSION_ID key)
{
	//패킷 조립 및 처리
	players[key].recvExOver.wsaBuf[0].buf = reinterpret_cast<CHAR*>(players[key].recvExOver.packetBuffer + players[key].prevSize);
	players[key].recvExOver.wsaBuf[0].len = MAX_BUFFER - players[key].prevSize;
	DWORD recvFlag = 0;
	memset(&players[key].recvExOver.over, 0, sizeof(WSAOVERLAPPED));
	int ret = WSARecv(players[key].socket, players[key].recvExOver.wsaBuf, 1, NULL, &recvFlag, &players[key].recvExOver.over, NULL);
	if ( ret != 0 )
	{
		int err_no = WSAGetLastError();
		if ( WSA_IO_PENDING != err_no )
		{
			display_error("Error in RecvPacket : ", err_no);
		}
	}
}

void KG::Server::Server::ProcessPacket(SESSION_ID playerId, unsigned char* buffer)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(buffer);
	auto it = this->netObjects.find(header->objectId);
	if ( it == this->netObjects.end() )
	{
		std::cout << "Unknown Packet Receiver Object : " << header->objectId << '\n';
	}
	else
	{
		bool processed = it->second->ProcessPacket(buffer, KG::Packet::ToPacketType(header->type), playerId);
		if ( !processed )
		{
			std::cout << "Packet Owner Object Not Processd / Object ID : " << header->objectId << " / PacketType : " << header->type << "\n";
		}
	}
}

void KG::Server::Server::SetSessionState(SESSION_ID session, KG::Server::PLAYER_STATE state)
{
	this->players[session].state = state;
}

void KG::Server::Server::SetSessionId(SESSION_ID session, KG::Server::NET_OBJECT_ID id)
{
	this->players[session].netId = id;
}

void KG::Server::Server::SetPlayerNum(int num) {
	this->playernum = num;
}

int KG::Server::Server::GetPlayerNum() {
	return this->playernum;
}

KG::Server::NET_OBJECT_ID KG::Server::Server::GetNewObjectId()
{
	std::lock_guard lg{ this->objectIdStartMutex };
	int ret = this->objectIdStart;
	this->objectIdStart += 1;
	return ret;
}


