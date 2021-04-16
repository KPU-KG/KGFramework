#include "pch.h"
#include "Protocol.h"
#include "ServerUtill.h"
#include "KGServer.h"


void KG::Server::Server::IOCPWorker(Server* server)
{
	while ( true )
	{
		DWORD numBytes;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(server->hIocp, &numBytes, &ikey, &over, INFINITE);
		int key = static_cast<int>(ikey);
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
				Disconnect(key);
				continue;
			}
		}

		if ( numBytes == 0 && key != 0 ) // 정상 종료
		{
			Disconnect(key);
		}

		EX_OVERLAPPED* ex_over = reinterpret_cast<EX_OVERLAPPED*>(over);
		switch ( ex_over->op )
		{
			case OP_RECV:
			{

				unsigned char* packet_ptr = ex_over->packetBuffer;
				int num_data = numBytes + players[key].prevSize;
				int packetSize = packet_ptr[0];

				while ( num_data >= packetSize ) // 다받음
				{
					ProcessPacket(key, packet_ptr);
					num_data -= packetSize;
					packet_ptr += packetSize;
					if ( 0 >= num_data ) break;
					packetSize = packet_ptr[0];
				}
				players[key].prevSize = num_data;
				if ( num_data != 0 )
				{
					memcpy(ex_over->packetBuffer, packet_ptr, num_data);
				}
				DoRecv(key);
			}
			break;
			case OP_SEND:
			{
				delete ex_over;
			}
			break;
			case OP_ACCEPT:
			{
				int clientId = GetNewPlayerId(ex_over->csocket);
				if ( -1 != clientId )
				{
					players[clientId].recvExOver.op = OP_RECV;
					players[clientId].prevSize = 0;
					DWORD flags = 0;
					CreateIoCompletionPort(reinterpret_cast<HANDLE>(ex_over->csocket), hIocp, clientId, NULL); //소켓 등록
					DoRecv(clientId);
				}
				else
				{
					closesocket(ex_over->csocket);
				}

				ex_over->op = OP_ACCEPT;
				memset(&ex_over->over, 0, sizeof(ex_over->over));
				ex_over->csocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				AcceptEx(listenSocket, ex_over->csocket, ex_over->packetBuffer, 0, 32, 32, 0, &ex_over->over);
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
}

void KG::Server::Server::Start()
{
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	EX_OVERLAPPED acceptOver;
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

void KG::Server::Server::GetNewPlayerNetworkController()
{
}

void KG::Server::Server::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
}

//Worker 
KG::Server::Server::SESSION_ID KG::Server::Server::GetNewPlayerId()
{
	std::lock_guard lg{ this->idStartMutex };
	int ret = this->idStart;
	this->idStart += 1;
	return ret;
}

void KG::Server::Server::SendPacket(SESSION_ID playerId, void* packet)
{
}

void KG::Server::Server::SendLoginOkPacket(SESSION_ID playerId)
{
}

void KG::Server::Server::SendWorldState(SESSION_ID playerId)
{
}

void KG::Server::Server::SendRemovePlayer(SESSION_ID playerId, SESSION_ID targetId)
{
}

void KG::Server::Server::SendAddPlayer(SESSION_ID playerId, SESSION_ID targetId)
{
}

void KG::Server::Server::Disconnect(SESSION_ID playerId)
{
}

void KG::Server::Server::DoRecv(SESSION_ID key)
{
}

void KG::Server::Server::ProcessPacket(SESSION_ID playerId, unsigned char* buffer)
{
}

