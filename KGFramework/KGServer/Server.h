#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <array>
#include "ComponentProvider.h"
#include "Scene.h"
#include "Data.h"
using namespace std;
struct Systems;

class Server {
	KG::Component::ComponentProvider componentProvider;
	KG::Core::Scene scene;
	HANDLE h_iocp;
	SOCKET listenSocket;
	DWORD num_bytes;
	ULONG_PTR ikey;
	WSAOVERLAPPED* over;

	array <SESSION, MAX_USER + 1> players;
	SYSTEM_INFO SystemInfo;

public:
	void Initialize();
	void InitServer();
	void CALLBACK Send_packet(int p_id, void* p);
	void Do_recv(int key);
	void Process_packet(int p_id, unsigned char* p_buf);
	void CompletionFunc();
	void UpdateThread();
	void Send_scene(int p_id);
	void Disconnect(int p_id);
	int Get_new_player_id(SOCKET p_socket);
	void display_error(const char* msg, int err_no);
};
