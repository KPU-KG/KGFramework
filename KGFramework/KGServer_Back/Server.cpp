#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <windows.h>
#include <mswsock.h>
#include <mutex>
#include <Mmsystem.h> 
#include <thread>
#include "Server.h"
#include "Data.h"
#define WIN32_LEAN_AND_MEAN
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib,"mswsock.lib")

void Server::Initialize() {
}

using namespace std;

int Server::Get_new_player_id(SOCKET p_socket) {
	for (int i = SERVER_ID + 1; i < MAX_USER; i++)
	{
		lock_guard<mutex> lg{ players[i].m_slock };
		if (PLST_FREE == players[i].m_state) {
			players[i].m_state = PLST_CONNECTED;
			players[i].m_socket = p_socket;
			return i;
		}
	}
	return -1;
}

void Server::UpdateThread() {
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
		if (delta >= 1.0f) {
			cout << "update" << endl;
			delta = 0;
		}

		lastTime = currTime;
	}
}

void Server::CompletionFunc() {
	while (true) {
		DWORD num_bytes;
		ULONG_PTR ikey;
		WSAOVERLAPPED* over;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &ikey, &over, INFINITE);

		int key = static_cast<int>(ikey);

		if (FALSE == ret)
		{
			if (SERVER_ID == key) {
				display_error("GQCS: ", WSAGetLastError());
				exit(-1);

			}
			else {
				display_error("GQCS: ", WSAGetLastError());
				Disconnect(key);
			}
		}
		if ((key != 0) && (0 == num_bytes)) {
			Disconnect(key);
			continue;
		}
		EX_OVER* ex_over = reinterpret_cast<EX_OVER*>(over);
		switch (ex_over->m_op)
		{
		case OP_RECV: {
			// 패킷 재조립 및 처리
			unsigned char* packet_ptr = ex_over->m_packetbuf;
			int num_data = num_bytes + players[key].m_prev_size;
			int packet_size = packet_ptr[0];

			while (num_data >= packet_size) {
				Process_packet(key, packet_ptr);
				num_data -= packet_size;
				packet_ptr += packet_size;
				if (0 >= num_data)   break;
				packet_size = packet_ptr[0];
			}

			players[key].m_prev_size = num_data;

			if (num_data != 0) {
				memcpy(ex_over->m_packetbuf, packet_ptr, num_data);
			}
			Do_recv(key);
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		case OP_ACCEPT: {
			int c_id = Get_new_player_id(ex_over->m_csocket);
			if (-1 != c_id) {

				players[c_id].m_recv_over.m_op = OP_RECV;
				players[c_id].m_prev_size = 0;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(players[c_id].m_socket), h_iocp, c_id, 0);

				Do_recv(c_id);
			}
			else {
				closesocket(players[c_id].m_socket);
			}

			memset(&ex_over->m_over, 0, sizeof(ex_over->m_over));
			SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			ex_over->m_csocket = c_socket;
			AcceptEx(listenSocket, c_socket, ex_over->m_packetbuf, 0, 32, 32, NULL, &ex_over->m_over);
		}
					  break;
		}
	}
}

void CALLBACK Server::Send_packet(int p_id, void* p)
{
	int p_size = reinterpret_cast<unsigned char*>(p)[0];
	int p_type = reinterpret_cast<unsigned char*>(p)[1];
	EX_OVER* s_over = new EX_OVER;
	s_over->m_op = OP_SEND;
	memset(&s_over->m_over, 0, sizeof(s_over->m_over));
	memcpy(s_over->m_packetbuf, p, p_size);
	s_over->m_wsabuf[0].buf = reinterpret_cast<CHAR*>(s_over->m_packetbuf);
	s_over->m_wsabuf[0].len = p_size;
	auto ret = WSASend(players[p_id].m_socket, s_over->m_wsabuf, 1, NULL, 0, &s_over->m_over, 0);
	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("WSASend:", err_no);
	}
}

void Server::Do_recv(int key) {
	players[key].m_recv_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(players[key].m_recv_over.m_packetbuf) + players[key].m_prev_size;
	players[key].m_recv_over.m_wsabuf[0].len = MAX_BUFFER - players[key].m_prev_size;
	DWORD recv_flag = 0;
	auto ret = WSARecv(players[key].m_socket, players[key].m_recv_over.m_wsabuf, 1, NULL, &recv_flag, &players[key].m_recv_over.m_over, NULL);

	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("WSASend:", err_no);
	}
}

void Server::display_error(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << lpMsgBuf << std::endl;
	while (true);
	LocalFree(lpMsgBuf);
}

void Server::Send_scene(int p_id) {
	SCENE_DATA p;
	p.size = sizeof(p);
	//
	p.type = S2C_SCENE_DATA;
	Send_packet(p_id, &p);
}

void Server::Process_packet(int p_id, unsigned char* p_buf) {
	switch (p_buf[1])
	{
	case C2S_INPUT: {
		//INPUT* packet = reinterpret_cast<INPUT*>(p_buf);
		INPUT_DATA* packet = reinterpret_cast<INPUT_DATA*>(p_buf);
		Send_scene(p_id);
	}
				  break;
	case C2S_FIRE:
		break;
	default:
		cout << "unknown packet type from client[" << p_id << "] packet type [" << p_buf[1] << "]" << endl;
		while (true);
		break;
	}
}

void Server::Disconnect(int p_id) {
	{
		lock_guard<mutex> gl{ players[p_id].m_slock };
		closesocket(players[p_id].m_socket);
		players[p_id].m_state = PLST_FREE;
	}
	for (auto& pl : players) {
		lock_guard<mutex> gl2{ pl.m_slock };
		//if (PLST_INGAME == pl.m_state)
			//send_remove_player(pl.id, p_id);
	}
}

void Server::InitServer() {
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, 0, 0);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	EX_OVER accept_over;
	accept_over.m_op = OP_ACCEPT;
	memset(&accept_over.m_over, 0, sizeof(accept_over.m_over));
	GetSystemInfo(&SystemInfo);


	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	accept_over.m_csocket = c_socket;
	BOOL ret = AcceptEx(listenSocket, c_socket, accept_over.m_packetbuf, 0, 32, 32, NULL, &accept_over.m_over);
	if (FALSE == ret) {
		int err_num = WSAGetLastError();
		if (err_num != WSA_IO_PENDING)
			display_error("AcceptEX Error: ", err_num);
	}
	vector<thread> Threads;
	Threads.emplace_back(thread{ &Server::UpdateThread, this });
	for (size_t i = 0; i < SystemInfo.dwNumberOfProcessors - 2; i++)
	{
		Threads.emplace_back(thread{ &Server::CompletionFunc, this});
	}
	for (auto& th : Threads)
		th.join();

	closesocket(listenSocket);
	WSACleanup();
}