#include "pch.h"
#include "Protocol.h"
#include "ServerUtill.h"
#include "KGServer.h"


DLL KG::Server::IServer* KG::Server::GetServer()
{
	return new Server();
}


void KG::Server::Server::IOCPWorker(Server* server)
{
	std::cout << "IOCP Worker Started\n";
	while ( true )
	{
		DWORD numBytes;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;
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
					newSession.state = PLAYER_STATE_INGAME;
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

	//Init Systems
	this->sGameManagerSystem.SetServerInstance(this);
}

void KG::Server::Server::Start()
{
	std::cout << "Start Server Process\n";
	this->LockWorld();
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	memset(&acceptOver.over, 0, sizeof(acceptOver.over));
	acceptOver.op = OP_ACCEPT;
	acceptOver.csocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);;
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
}

void KG::Server::Server::UnlockWorld()
{
	worldLock.unlock();
}

KG::Component::SGameManagerComponent* KG::Server::Server::GetNewGameManagerComponent()
{
	auto* comp = this->sGameManagerSystem.GetNewComponent();
	comp->SetServerInstance(this);
	return comp;
}

void KG::Server::Server::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
	this->sGameManagerSystem.OnPostProvider(provider);
}

void KG::Server::Server::DrawImGUI()
{
	static char ipBuffer[256] = "127.0.0.1";
	if ( ImGui::CollapsingHeader("Network Server", ImGuiTreeNodeFlags_DefaultOpen) )
	{
		if ( !this->isStarted() )
		{
			if ( ImGui::Button("Start") )
			{
				this->Start();
			}
		}
	}
}

bool KG::Server::Server::isStarted() const
{
	return this->iocpWorkers.size() != 0;
}

//Worker 
KG::Server::SESSION_ID KG::Server::Server::GetNewPlayerId()
{
	std::lock_guard lg{ this->idStartMutex };
	int ret = this->idStart;
	this->idStart += 1;
	return ret;
}

void KG::Server::Server::BroadcastPacket(void* packet, SESSION_ID ignore)
{
	std::cout << "BroadCast Packet!\n";
	for ( auto& i : this->players )
	{
		if ( i.first != ignore )
		{
			std::shared_lock<std::shared_mutex> lg{ i.second.sessionLock };
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

	std::cout << "To client [" << playerId << "] : ";
	std::cout << "Packet / Type : " << packetType << " / Size : " << packetSize << "\n";

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

void KG::Server::Server::SendLoginOkPacket(SESSION_ID playerId)
{
}

void KG::Server::Server::SendWorldState(SESSION_ID playerId)
{
}

void KG::Server::Server::SendRemovePlayer(SESSION_ID playerId, SESSION_ID targetId)
{
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
	{
		std::lock_guard<std::shared_mutex> lg{ players[playerId].sessionLock };
		players[playerId].state = PLAYER_STATE_FREE;
		closesocket(players[playerId].socket);
	}

	for ( auto& [id , pl] : players )
	{
		std::shared_lock<std::shared_mutex> lg2{ pl.sessionLock };
		if ( pl.state == PLAYER_STATE_INGAME )
		{
			SendRemovePlayer(playerId, pl.id);
		}
	}

	std::cout << "Disconnected Client[" << playerId << "]\n";
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
	switch ( buffer[1] ) // Packet Type
	{
		//case CS_LOGIN:
		//{
		//	auto* packet = reinterpret_cast<packet_c2s_login*>(buffer);
		//	strcpy_s(players[playerId].name, packet->name);
		//	SendLoginOkPacket(playerId);

		//	lock_guard<mutex> lg{ players[playerId].sessionLock };
		//	players[playerId].state = PLAYERSTATE_INGAME;
		//	for ( auto& pl : players )
		//	{
		//		if ( playerId == pl.id ) continue;

		//		lock_guard<mutex> lg2{ pl.sessionLock };
		//		if ( pl.state == PLAYERSTATE_INGAME )
		//		{
		//			SendAddPlayer(playerId, pl.id);
		//			SendAddPlayer(pl.id, playerId);
		//		}

		//	}
		//}
		//break;
		//case CS_MOVE:
		//{
		//	auto* packet = reinterpret_cast<packet_c2s_move*>(buffer);
		//	DoMove(playerId, packet->dir);
		//}
		//break;
		default:
			std::cout << "UNKNOWN Packet Type From Client[" << playerId << "] Packet Type [" << buffer[1] << "]\n";
			while ( true );
			break;
	}
}

