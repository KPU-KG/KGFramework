#pragma once
#include <mutex>

constexpr unsigned char C2S_CONNECT = 1;
constexpr unsigned char C2S_READY_ON = 2;
constexpr unsigned char C2S_READY_OFF = 3;
constexpr unsigned char C2S_MODE_SELECT = 4;
constexpr unsigned char C2S_EXIT = 5;

constexpr unsigned char C2S_INPUT = 6;
constexpr unsigned char C2S_FIRE = 7;

constexpr unsigned char S2C_ROOM_DATA = 8;
constexpr unsigned char S2C_SCENE_DATA = 9;

constexpr int MAX_NAME = 100;
constexpr int MAX_BUFFER = 1024;
constexpr short SERVER_PORT = 3500;
constexpr int MAX_USER = 100;
constexpr int SERVER_ID = 0;


enum OP_TYPE {
	OP_RECV,
	OP_SEND,
	OP_ACCEPT,
};

enum PL_STATE {
	PLST_FREE,
	PLST_CONNECTED,
	PLST_INGAME
};

struct EX_OVER
{
	WSAOVERLAPPED m_over;
	WSABUF m_wsabuf[1];
	unsigned char m_packetbuf[MAX_BUFFER];
	OP_TYPE m_op;
	SOCKET m_csocket;
};

struct SESSION
{
	std::mutex m_slock;
	PL_STATE m_state;
	SOCKET m_socket;
	int id;

	EX_OVER m_recv_over;
	int m_prev_size;

	//char m_name[MAX_NAME];
	//short x, y;
};



#pragma pack(push, 1)

struct CONNECT {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};

struct READY_ON {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};

struct READY_OFF {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};

struct MODE_SELECT {
	unsigned char size;
	unsigned char type;
	bool mode;
};

struct EXIT {
	unsigned char size;
	unsigned char type;
	unsigned short id;
};

///////////////
struct INPUT_DATA {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	//bool input[];
};

struct FIRE {
	unsigned char size;
	unsigned char type;
	unsigned short id;
	//
};

#pragma pack(pop)

struct ROOM_DATA {
	unsigned char size;
	unsigned char type;
	// 방 정보
};

struct SCENE_DATA {
	unsigned char size;
	unsigned char type;
	// 씬 정보
};

