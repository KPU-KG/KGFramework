#pragma once
#include <iostream>
#include <WS2tcpip.h>
#include <thread>
#include <map>
#include "Data.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class Server {
	HANDLE	iocp_Handle;
	SOCKET listenSocket;
	SOCKADDR_IN serverAddr;
	SOCKADDR_IN clientAddr;
	map <SOCKET, SOCKETINFO> clients;

	LPPER_IO_DATA io_data;
	LPPER_HANDLE_DATA overlapped_data;
	int ConcurrentUsers = 0;

	WSADATA wsaData;
	SYSTEM_INFO SystemInfo;

	InputData gamedata;
	Input_Datas server_data;
	
public:
	/*void WorkerThread();
	unsigned int __stdcall WorkerrThread();*/
	void ServerInit();
	void CloseSocket(SOCKET client_socket);
	void SetServer(SOCKADDR_IN& serverAddr, int network, int port);
	void InitListenSock();
	InputData Getdata() { return gamedata; };
	void Setdata(InputData data) { gamedata = data; };
	DWORD SetClient(SOCKET clientSocket);

	void error_display(const char* msg, int err_no)
	{
		WCHAR* lpMsgBuf;
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, err_no,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);
		cout << msg;
		wcout << L"¿¡·¯ " << lpMsgBuf << endl;
		while (true);
		LocalFree(lpMsgBuf);
	}

};