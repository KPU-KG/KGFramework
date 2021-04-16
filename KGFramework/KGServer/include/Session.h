#pragma once
#include "Protocol.h"
#include <mutex>
#include <shared_mutex>
#include <WS2tcpip.h>
#include <MSWSock.h>

namespace KG::Server
{
	enum OP_TYPE
	{
		OP_RECV,
		OP_SEND,
		OP_ACCEPT
	};

	enum PLAYER_STATE
	{
		PLAYER_STATE_FREE,
		PLAYER_STATE_CONNECTED,
		PLAYER_STATE_INGAME
	};

	struct EX_OVERLAPPED
	{
		WSAOVERLAPPED over;
		WSABUF wsaBuf[1];
		unsigned char packetBuffer[MAX_BUFFER];
		OP_TYPE op;
		SOCKET csocket; // Only OP_ACCEPT
	};

	struct SESSION
	{
		std::shared_mutex sessionLock;
		PLAYER_STATE state;

		// Network Info
		SOCKET socket;
		int id;
		EX_OVERLAPPED recvExOver;
		int prevSize;

		// Game Content Part
		//KG::Core::GameObject* playerObject; // or PlayerController;
	};


};
