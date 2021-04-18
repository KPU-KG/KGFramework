#include <thread>
#include "server.h"
#include <Mmsystem.h> 
#include "MathHelper.h"
#include "Data.h"

#include <mswsock.h>
#pragma comment(lib,"mswsock.lib")

using namespace std;

//Input_Datas input_datas;
//std::vector<PlayerData> PlayerDatas;

int ServerComponent::Get_new_player_id() {
	for (size_t i = SERVER_ID + 1; i < MAX_USER; i++)
	{
		if (0 == players.count(i)) return i;
	}
}

void ServerComponent::UpdateThread() {
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
			//for (size_t i = 0; i < PlayerDatas.size(); i++)
			//{
			//	tx = 0, ty = 0;
			//	if (input_datas.data[i].isWClicked) {
			//		tx += 1.0f;
			//		//cout << i << "Client move foward" << endl;
			//	}
			//	if (input_datas.data[i].isSClicked) {
			//		tx -= 1.0f;
			//		//cout << i << "Client move back" << endl;
			//	}
			//	if (input_datas.data[i].isAClicked) {
			//		ty -= 1.0f;
			//		//cout << i << "Client move left" << endl;
			//	}
			//	if (input_datas.data[i].isDClicked) {
			//		ty += 1.0f;
			//		//cout << i << "Client move right" << endl;
			//	}

			//	PlayerDatas[i].position = Math::Vector3::Add(PlayerDatas[i].position, delta * ty * speed * Math::GetRight(PlayerDatas[i].rotation));
			//	PlayerDatas[i].position = Math::Vector3::Add(PlayerDatas[i].position, delta * tx * speed * Math::GetLook(PlayerDatas[i].rotation));

			//	if (input_datas.data[i].is1Clicked) {
			//		PlayerDatas[i].currentAnimation = 'l';
			//	}
			//	else if (input_datas.data[i].is2Clicked) {
			//		PlayerDatas[i].currentAnimation = 'f';
			//	}
			//	else if (input_datas.data[i].is3Clicked) {
			//		PlayerDatas[i].currentAnimation = 'r';
			//	}
			//	else {
			//		PlayerDatas[i].currentAnimation = ' ';
			//	}
			//}
			cout << "update" << endl;
			delta = 0;
		}

		lastTime = currTime;
	}
}

void ServerComponent::CompletionFunc() {
	while (1)
	{
		int key = static_cast<int>(ikey);
		EX_OVER* ex_over = reinterpret_cast<EX_OVER*>(over);

		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &ikey, &over, INFINITE);

		if (ex_over->m_op) { // recv
			unsigned char* packet_ptr = ex_over->m_packetbuf;
			int num_data = num_bytes + players[key].m_prev_size;
			int packet_size = packet_ptr[0];
			while (num_data >= packet_size) {
				Process_packet(key, packet_ptr);
				num_data -= packet_size;
				packet_ptr += packet_size;
				if (0 >= num_data) break;
				packet_size = packet_ptr[0];
			}

			players[key].m_prev_size = num_data;

			if (num_data != 0) {
				memcpy(ex_over->m_packetbuf, packet_ptr, num_data);
			}
			Do_recv(key);
		}
		else { 
			delete ex_over;
		}
	}
	return;
}

void CALLBACK ServerComponent::Send_packet(int p_id, void* p)
{
	int p_size = reinterpret_cast<unsigned char*>(p)[0];
	int p_type = reinterpret_cast<unsigned char*>(p)[1];
	//cout << "To client [" << p_id << "]: ";
	//cout << "Packet [" << p_type << "]\n";
	EX_OVER* s_over = new EX_OVER;
	s_over->m_op = false;
	memset(&s_over->m_over, 0, sizeof(s_over->m_over));
	memcpy(s_over->m_packetbuf, p, p_size);
	s_over->m_wsabuf[0].buf = reinterpret_cast<CHAR*>(s_over->m_packetbuf);
	s_over->m_wsabuf[0].len = p_size;
	int ret = WSASend(players[p_id].socket, s_over->m_wsabuf, 1, NULL, 0, &s_over->m_over, 0);
	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("WSASend:", err_no);
	}
}

void ServerComponent::Do_recv(int key) {
	players[key].m_recv_over.m_wsabuf[0].buf = reinterpret_cast<CHAR*>(players[key].m_recv_over.m_packetbuf) + players[key].m_prev_size;
	players[key].m_recv_over.m_wsabuf[0].len = MAX_BUFFER - players[key].m_prev_size;
	memset(&players[key].m_recv_over.m_over, 0, sizeof(players[key].m_recv_over.m_over));
	DWORD recv_flag = 0;
	int ret = WSARecv(players[key].socket, players[key].m_recv_over.m_wsabuf, 1,
		NULL, &recv_flag, &players[key].m_recv_over.m_over, NULL);
	if (ret != 0) {
		int err_no = WSAGetLastError();
		if (WSA_IO_PENDING != err_no)
			display_error("WSASend:", err_no);
	}
}

void ServerComponent::display_error(const char* msg, int err_no)
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

void ServerComponent::Send_scene(int p_id) {
	SCENE_DATA p;
	p.size = sizeof(p);
	//
	p.type = S2C_SCENE_DATA;
	Send_packet(p_id, &p);
}

void ServerComponent::Process_packet(int p_id, unsigned char* p_buf) {
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

void ServerComponent::Disconnect(int p_id) {
	closesocket(players[p_id].socket);
	players.erase(p_id);
}

void ServerComponent::Loop() {
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), h_iocp, 0, 0);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));
	listen(listenSocket, SOMAXCONN);

	EX_OVER accept_over;
	accept_over.m_op = true;
	memset(&accept_over.m_over, 0, sizeof(accept_over.m_over));
	GetSystemInfo(&SystemInfo);

	//_beginthreadex(NULL, 0, UpdateThread, (LPVOID)iocp_Handle, 0, NULL);
	//for (int i = 0; i < SystemInfo.dwNumberOfProcessors; i++)
	//	_beginthreadex(NULL, 0, &CompletionThread, this, 0, NULL);

	vector<thread> Threads;
	Threads.emplace_back(thread{ &ServerComponent::UpdateThread, this });
	for (size_t i = 0; i < SystemInfo.dwNumberOfProcessors - 2; i++)
	{
		Threads.emplace_back(thread{ &ServerComponent::CompletionFunc, this });
	}

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	while (true) { // Accept
		int c_id = Get_new_player_id();
		if (-1 != c_id) {
			players[c_id] = SESSION{};
			players[c_id].id = c_id;
			players[c_id].m_recv_over.m_op = true; // recv
			players[c_id].socket = c_socket;
			players[c_id].m_prev_size = 0;
			CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, c_id, 0);

			memset(&accept_over.m_over, 0, sizeof(accept_over.m_over));
			//c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			AcceptEx(listenSocket, c_socket, accept_over.m_packetbuf, 0, 32, 32, NULL, &accept_over.m_over);
		}
		else {
			closesocket(c_socket);
		}

	}
	closesocket(listenSocket);
	WSACleanup();
}