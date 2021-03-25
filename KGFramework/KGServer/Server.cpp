//#include "Server.h"
//#include <vector>
//
//using namespace std;
//std::vector<PlayerData> PlayerDatas;
//static User_Datas userDatas;
//static bool client[MAX_Client] = { false, }; // 클라이언트가 들어오는 대로 처리
//
//
//unsigned int __stdcall CompletionThread(LPVOID pComPort) {
//
//	HANDLE hCompletionPort = (HANDLE)pComPort;
//	DWORD BytesTransferred;
//	LPPER_HANDLE_DATA PerHandleData;
//	LPPER_IO_DATA PerIoData;
//	DWORD flags;
//
//	while (1)
//	{
//		GetQueuedCompletionStatus(hCompletionPort,    // Completion Port
//			&BytesTransferred,   // 전송된 바이트수
//			(LPDWORD)&PerHandleData,
//			(LPOVERLAPPED*)&PerIoData, // OVERLAPPED 구조체 포인터.
//			INFINITE
//		);
//		
//		if (BytesTransferred == 0) {
//			printf("- 클라 종료 -\n");
//			client[PerHandleData->client_id] = false;
//			closesocket(PerHandleData->hClntSock);
//			free(PerHandleData);
//			free(PerIoData);
//			continue;
//		}
//		if (PerIoData->Incoming_data == Recv_Mode) {
//			PerIoData->wsaBuf.buf[BytesTransferred] = '\0';
//			userDatas.input.data[PerHandleData->client_id] = (InputData&)PerIoData->buffer;
//			PerIoData->Incoming_data = Send_Mode;
//		}
//		if (PerIoData->Incoming_data == Send_Mode) {
//			PerIoData->wsaBuf.len = sizeof(User_Datas);
//			PerIoData->wsaBuf.buf = (char*)&userDatas;
//			PerIoData->Incoming_data = Recv_Mode;
//			WSASend(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, NULL, 0, NULL, NULL);
//			
//			// 벡터 데이터 전송
//			// 클라측은 사이즈 데이터 받고, 해당 사이즈만큼 데이터 수신
//		}
//
//		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
//		PerIoData->wsaBuf.len = MAX_BUFFER;
//		PerIoData->wsaBuf.buf = PerIoData->buffer;
//
//		flags = 0;
//		WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, NULL, &flags, &(PerIoData->overlapped), NULL);
//	}
//	return 0;
//}
//
//unsigned int __stdcall UpdateThread(LPVOID pComPort) {
//	DWORD lastTime = timeGetTime();
//	DWORD currTime;
//	float delta = 0;
//	float tx = 0;
//	float ty = 0;
//	float speed = 2.0f;
//	float val = 0;
//	while (true) {
//		currTime = timeGetTime();
//		delta += (currTime - lastTime) * 0.001f;
//		
//		if (delta >= 0.05f) {
//			for (size_t i = 0; i < MAX_Client; i++)
//			{
//				tx = 0, ty = 0;
//				if (userDatas.input.data[i].isWClicked) {
//					tx += 1.0f;
//					//cout << i << "Client move foward" << endl;
//				}
//				if (userDatas.input.data[i].isSClicked) {
//					tx -= 1.0f;
//					//cout << i << "Client move back" << endl;
//				}
//				if (userDatas.input.data[i].isAClicked) {
//					ty -= 1.0f;
//					//cout << i << "Client move left" << endl;
//				}
//				if (userDatas.input.data[i].isDClicked) {
//					ty += 1.0f;
//					//cout << i << "Client move right" << endl;
//				}
//				
//				// 노멀라이즈 필요
//				userDatas.playerdata.data[i].position = Math::Vector3::Add(userDatas.playerdata.data[i].position, delta * ty * speed * Math::GetRight(userDatas.playerdata.data[i].rotation));
//				userDatas.playerdata.data[i].position = Math::Vector3::Add(userDatas.playerdata.data[i].position, delta * tx * speed * Math::GetLook(userDatas.playerdata.data[i].rotation));
//
//				if (userDatas.input.data[i].is1Clicked) {
//					userDatas.playerdata.data[i].currentAnimation = 'l';
//				}
//				else if (userDatas.input.data[i].is2Clicked) {
//					userDatas.playerdata.data[i].currentAnimation = 'f';
//				}
//				else if (userDatas.input.data[i].is3Clicked) {
//					userDatas.playerdata.data[i].currentAnimation = 'r';
//				}
//				else {
//					userDatas.playerdata.data[i].currentAnimation = ' ';
//				}
//
//			}
//			
//			delta = 0;
//		}
//
//		lastTime = currTime;
//	}
//}
//
//
//void Server::CloseSocket(SOCKET client_socket) {
//	ConcurrentUsers -= 1;
//	closesocket(client_socket);
//	clients.erase(client_socket);
//}
//
////void Server::WorkerThread()
////{
////
////	while (true) {
////		DWORD data_size;
////		ULONG key;
////		WSAOVERLAPPED* p_over;
////		DWORD flags;
////		// update만 시간따라, 통신은 패킷 수신했을 때만
////		// 만약 수신이 오랫동안 안됨 -> 연결 해제?
////
////		GetQueuedCompletionStatus(g_iocp, &data_size, &key, &p_over, INFINITE);
////
////		OVERLAPPED_EX* overlapped = reinterpret_cast<OVERLAPPED_EX*> (p_over);
////		SOCKET client_socket = static_cast<SOCKET>(key);
////
////		if (data_size == 0) {
////			CloseSocket(client_socket);
////			delete p_over;
////			continue;
////		}  // 클라이언트가 closesocket을 했을 경우
////
////		if (overlapped->is_recv) {
////			overlapped->wsabuf.buf[data_size] = '\0';
////			Setdata((InputData&)overlapped->wsabuf.buf);
////			overlapped->is_recv = false;
////
////			OVERLAPPED_EX* send_over = new OVERLAPPED_EX;
////
////			if (data_size == 0) {
////				CloseSocket(client_socket);
////				continue;
////			}  // closesocket
////
////		}
////		else {
////			overlapped->wsabuf.len = sizeof(InputData);
////			overlapped->wsabuf.buf = (char*)&gamedata;
////			WSASend(client_socket, &overlapped->wsabuf, 1, NULL, 0, NULL, NULL);
////		}
////
////		memset(&overlapped->over, 0x00, sizeof(WSAOVERLAPPED));
////		flags = 0;
////		WSARecv(client_socket, &overlapped->wsabuf, 1, 0, &flags, &overlapped->over, 0);
////	}
////}
//
//void Server::SetServer(SOCKADDR_IN& serverAddr, int network, int port) {
//	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
//	serverAddr.sin_family = network;
//	serverAddr.sin_port = htons(port);
//	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//}
//
//DWORD Server::SetClient(SOCKET clientSocket) {
//	ConcurrentUsers += 1;
//	clients[clientSocket] = SOCKETINFO{};
//	memset(&clients[clientSocket], 0, sizeof(struct SOCKETINFO));
//	clients[clientSocket].socket = clientSocket;
//	clients[clientSocket].recv_over.wsabuf.len = MAX_BUFFER;
//	clients[clientSocket].recv_over.wsabuf.buf = clients[clientSocket].recv_over.net_buf;
//	clients[clientSocket].recv_over.is_recv = true;
//	CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocpHandle, clientSocket, 0);
//	return 0;
//}
//
//void Server::InitListenSock() {
//	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); // STREAM   -TCP  DGRAM - UDP
//	::bind(listenSocket, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN));
//	listen(listenSocket, 5);
//}
//
//void Server::ServerInit() {
//	int RecvBytes;
//	int i, Flags;
//
//	//PlayerDatas.reserve(100);
//	//PlayerDatas.emplace_back();
//
//	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */ {}
//
//	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
//	GetSystemInfo(&SystemInfo);
//
//	_beginthreadex(NULL, 0, UpdateThread, (LPVOID)iocpHandle, 0, NULL);
//	SetServer(serverAddr, AF_INET, SERVER_PORT);
//	InitListenSock();
//	for (i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
//		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)iocpHandle, 0, NULL);
//
//	while (true)
//	{
//		SOCKET hClntSock;
//		SOCKADDR_IN clntAddr;
//		int addrLen = sizeof(clntAddr);
//
//		hClntSock = accept(listenSocket, (SOCKADDR*)&clntAddr, &addrLen);
//		PerHandleData = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
//		PerHandleData->hClntSock = hClntSock;
//		memcpy(&(PerHandleData->clntAddr), &clntAddr, addrLen);
//		CreateIoCompletionPort((HANDLE)hClntSock, iocpHandle, (DWORD)PerHandleData, 0);
//		
//		PerIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
//		for (int i = 0; i < MAX_Client; ++i) {
//			if (client[i] == false) {
//				client[i] = true;
//				PerHandleData->client_id = i;
//				printf("Client ID : %d\n", PerHandleData->client_id);
//				break;
//			}
//		}
//		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
//		PerIoData->wsaBuf.len = MAX_BUFFER;
//		PerIoData->wsaBuf.buf = PerIoData->buffer;
//		PerIoData->Incoming_data = Recv_Mode;
//
//		Flags = 0;
//		WSARecv(PerHandleData->hClntSock, &(PerIoData->wsaBuf), 1, (LPDWORD)&RecvBytes, (LPDWORD)&Flags, &(PerIoData->overlapped), NULL);
//	}
//	return;
//}


#include "Server.h"
#include <vector>

using namespace std;
//std::vector<User_Datas> user_Datas;
static User_Datas userDatas;
static bool client[MAX_Client] = { false, }; // 클라이언트가 들어오는 대로 처리


unsigned int __stdcall CompletionThread(LPVOID pComPort) {

	HANDLE hCompletionPort = (HANDLE)pComPort;
	DWORD BytesTransferred;
	LPPER_HANDLE_DATA PerHandleData;
	LPPER_IO_DATA PerIoData;
	DWORD flags;

	while (1)
	{
		GetQueuedCompletionStatus(hCompletionPort,    // Completion Port
			&BytesTransferred,   // 전송된 바이트수
			(PULONG_PTR)&PerHandleData,
			(LPOVERLAPPED*)&PerIoData, // OVERLAPPED 구조체 포인터.
			INFINITE
		);

		if (BytesTransferred == 0) {
			printf("- 클라 종료 -\n");
			client[PerHandleData->client_id] = false;
			closesocket(PerHandleData->client_handle);
			free(PerHandleData);
			free(PerIoData);
			continue;
		}
		if (PerIoData->incoming_data == Recv_Mode) {
			PerIoData->wsaBuf.buf[BytesTransferred] = '\0';
			userDatas.input.data[PerHandleData->client_id] = (InputData&)PerIoData->buffer;
			PerIoData->incoming_data = Send_Mode;
		}
		if (PerIoData->incoming_data == Send_Mode) {
			//PerIoData->wsaBuf.len = sizeof(User_Datas) * user_Datas.size();
			PerIoData->wsaBuf.len = sizeof(User_Datas);
			PerIoData->wsaBuf.buf = (char*)&userDatas;
			PerIoData->incoming_data = Recv_Mode;
			WSASend(PerHandleData->client_handle, &(PerIoData->wsaBuf), 1, NULL, 0, NULL, NULL);

			// 벡터 데이터 전송
			// 클라측은 사이즈 데이터 받고, 해당 사이즈만큼 데이터 수신
		}

		memset(&(PerIoData->overlapped), 0, sizeof(OVERLAPPED));
		PerIoData->wsaBuf.len = MAX_BUFFER;
		PerIoData->wsaBuf.buf = PerIoData->buffer;

		flags = 0;
		WSARecv(PerHandleData->client_handle, &(PerIoData->wsaBuf), 1, NULL, &flags, &(PerIoData->overlapped), NULL);
	}
	return 0;
}

unsigned int __stdcall UpdateThread(LPVOID pComPort) {
	DWORD lastTime = timeGetTime();
	DWORD currTime;
	float delta = 0;
	float tx = 0;
	float ty = 0;
	float speed = 2.0f;
	float val = 0;
	while (true) {
		currTime = timeGetTime();
		delta += (currTime - lastTime) * 0.001f;

		if (delta >= 0.05f) {
			for (size_t i = 0; i < MAX_Client; i++)
			{
				tx = 0, ty = 0;
				if (userDatas.input.data[i].isWClicked) {
					tx += 1.0f;
					//cout << i << "Client move foward" << endl;
				}
				if (userDatas.input.data[i].isSClicked) {
					tx -= 1.0f;
					//cout << i << "Client move back" << endl;
				}
				if (userDatas.input.data[i].isAClicked) {
					ty -= 1.0f;
					//cout << i << "Client move left" << endl;
				}
				if (userDatas.input.data[i].isDClicked) {
					ty += 1.0f;
					//cout << i << "Client move right" << endl;
				}

				// 노멀라이즈 필요
				userDatas.playerdata.data[i].position = Math::Vector3::Add(userDatas.playerdata.data[i].position, delta * ty * speed * Math::GetRight(userDatas.playerdata.data[i].rotation));
				userDatas.playerdata.data[i].position = Math::Vector3::Add(userDatas.playerdata.data[i].position, delta * tx * speed * Math::GetLook(userDatas.playerdata.data[i].rotation));

				if (userDatas.input.data[i].is1Clicked) {
					userDatas.playerdata.data[i].currentAnimation = 'l';
				}
				else if (userDatas.input.data[i].is2Clicked) {
					userDatas.playerdata.data[i].currentAnimation = 'f';
				}
				else if (userDatas.input.data[i].is3Clicked) {
					userDatas.playerdata.data[i].currentAnimation = 'r';
				}
				else {
					userDatas.playerdata.data[i].currentAnimation = ' ';
				}

			}

			delta = 0;
		}

		lastTime = currTime;
	}
}


void Server::CloseSocket(SOCKET client_socket) {
	ConcurrentUsers -= 1;
	closesocket(client_socket);
	clients.erase(client_socket);
}


void Server::SetServer(SOCKADDR_IN& serverAddr, int network, int port) {
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = network;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
}

DWORD Server::SetClient(SOCKET clientSocket) {
	ConcurrentUsers += 1;
	clients[clientSocket] = SOCKETINFO{};
	memset(&clients[clientSocket], 0, sizeof(struct SOCKETINFO));
	clients[clientSocket].socket = clientSocket;
	clients[clientSocket].recv_over.wsabuf.len = MAX_BUFFER;
	clients[clientSocket].recv_over.wsabuf.buf = clients[clientSocket].recv_over.net_buf;
	clients[clientSocket].recv_over.is_recv = true;
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocp_Handle, clientSocket, 0);
	return 0;
}

void Server::InitListenSock() {
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED); // STREAM   -TCP  DGRAM - UDP
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, 5);
}

void Server::ServerInit() {
	int RecvBytes;
	int i, Flags;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) /* Load Winsock 2.2 DLL */ {}

	iocp_Handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&SystemInfo);

	_beginthreadex(NULL, 0, UpdateThread, (LPVOID)iocp_Handle, 0, NULL);
	SetServer(serverAddr, AF_INET, SERVER_PORT);
	InitListenSock();
	for (i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
		_beginthreadex(NULL, 0, CompletionThread, (LPVOID)iocp_Handle, 0, NULL);

	while (true)
	{
		SOCKET client_handle;
		SOCKADDR_IN clntAddr;
		int addrLen = sizeof(clntAddr);

		client_handle = accept(listenSocket, (SOCKADDR*)&clntAddr, &addrLen);
		overlapped_data = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		overlapped_data->client_handle = client_handle;
		memcpy(&(overlapped_data->client_addr), &clntAddr, addrLen);
		CreateIoCompletionPort((HANDLE)client_handle, iocp_Handle, (DWORD)overlapped_data, 0);

		io_data = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		for (int i = 0; i < MAX_Client; ++i) {
			if (client[i] == false) {
				client[i] = true;
				overlapped_data->client_id = i;
				printf("Client ID : %d\n", overlapped_data->client_id);
				break;
			}
		}
		memset(&(io_data->overlapped), 0, sizeof(OVERLAPPED));
		io_data->wsaBuf.len = MAX_BUFFER;
		io_data->wsaBuf.buf = io_data->buffer;
		io_data->incoming_data = Recv_Mode;

		Flags = 0;
		WSARecv(overlapped_data->client_handle, &(io_data->wsaBuf), 1, (LPDWORD)&RecvBytes, (LPDWORD)&Flags, &(io_data->overlapped), NULL);
	}
	return;
}






