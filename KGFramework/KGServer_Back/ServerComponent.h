#pragma once
#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>
#include <thread>
#include <MSWSock.h>
#include "IComponent.h"
#include "ISerializable.h"
#include "SerializableProperty.h"
#include "Data.h"

namespace KG::Core
{
	class GameObject;
};

class ServerComponent : public KG::Component::IComponent {
	HANDLE h_iocp;
	DWORD num_bytes;
	ULONG_PTR ikey;
	WSAOVERLAPPED* over;

	std::unordered_map <int, SESSION> players;
	SYSTEM_INFO SystemInfo;

public:
	void Loop();
	void CALLBACK Send_packet(int p_id, void* p); 
	void Do_recv(int key);
	void Process_packet(int p_id, unsigned char* p_buf);
	void CompletionFunc(); 
	void UpdateThread();
	void Send_scene(int p_id);
	void Disconnect(int p_id);
	int Get_new_player_id();
	void display_error(const char* msg, int err_no);
};

REGISTER_COMPONENT_ID(ServerComponent);
