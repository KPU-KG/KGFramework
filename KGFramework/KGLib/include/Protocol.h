#pragma once

namespace KG::Server
{
	constexpr int MAX_NAME = 100;
	constexpr int MAX_BUFFER = 1024;
	constexpr short SERVER_PORT = 3500;
};
namespace KG::Packet
{
#pragma once
	namespace PacketId
	{
		constexpr unsigned char C2S_CONNECT = 1;
		constexpr unsigned char C2S_READY_ON = 2;
		constexpr unsigned char C2S_READY_OFF = 3;
		constexpr unsigned char C2S_MODE_SELECT = 4;
		constexpr unsigned char C2S_EXIT = 5;

		constexpr unsigned char C2S_INPUT = 6;
		constexpr unsigned char C2S_FIRE = 7;

		constexpr unsigned char S2C_ROOM_DATA = 8;
		constexpr unsigned char S2C_SCENE_DATA = 9;
	};

#pragma pack(push, 1)
	struct CONNECT
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
	};

	struct READY_ON
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
	};

	struct READY_OFF
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
	};

	struct MODE_SELECT
	{
		unsigned char size;
		unsigned char type;
		bool mode;
	};

	struct EXIT
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
	};

	///////////////
	struct INPUT_DATA
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
		//bool input[];
	};

	struct FIRE
	{
		unsigned char size;
		unsigned char type;
		unsigned short id;
		//
	};

	struct ROOM_DATA
	{
		unsigned char size;
		unsigned char type;
		// 방 정보
	};

	struct SCENE_DATA
	{
		unsigned char size;
		unsigned char type;
		// 씬 정보
	};
#pragma pack(pop)


};
