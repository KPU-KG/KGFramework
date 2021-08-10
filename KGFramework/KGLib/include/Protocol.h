#pragma once
#define DEFAULT_PACKET_HEADER( x ) PacketHeader header = PacketHeader{sizeof(x), PacketID(KG::Packet::PacketType::x), KG::Server::NULL_NET_OBJECT_ID }
#define PLAYERNUM 4

#include "hash.h"
#include <DirectXMath.h>

namespace KG::Server
{
	constexpr int MAX_NAME = 100;
	constexpr int MAX_BUFFER = 1024;
	constexpr short SERVER_PORT = 3500;
	using SESSION_ID = int;
	using NET_OBJECT_ID = int;
	constexpr SESSION_ID SERVER_ID = 0;
	constexpr NET_OBJECT_ID SCENE_CONTROLLER_ID = 0;
	constexpr NET_OBJECT_ID LOBBY_ID = 1;
	constexpr NET_OBJECT_ID NULL_NET_OBJECT_ID = -343434;

};

namespace KG::Packet
{
	enum class PacketType : unsigned char
	{
		None = 0,
		PacketHeader = 1, 

		SC_PLAYER_INIT,
		SC_ADD_OBJECT,
		SC_REMOVE_OBJECT,
		SC_SCALE_OBJECT,
		// SC_REMOVE_TEAM_CHARACTOR, 따로 있을 필요가 있을까?? 11일에 생각해봅시다..
		SC_REMOVE_PLAYER,
		SC_FIRE, 
		SC_ADD_PLAYER,
		SC_PLAYER_SYNC,
		SC_SCENE_DATA,
		SC_MOVE_OBJECT,
		SC_CHANGE_ANIMATION,
		SC_SYNC_ANIMATION,
		SC_PLAYER_DATA,
        SC_ENEMY_HP,
		SC_LOGIN_OK,
		SC_LOBBY_FULL,
		SC_LOBBY_DATA,
		SC_GAME_START,
		SC_GAME_END,
		CS_REQ_LOGIN, 
		CS_INPUT, // 사용
		CS_FIRE,
		CS_RELOAD,
		CS_LOBBY_CHANGE,
		CS_SELECT_MAP
	};

	
	constexpr unsigned char PacketID(PacketType type)
	{
		return static_cast<unsigned char>(type);
	}

	constexpr PacketType ToPacketType(unsigned char type)
	{
		return static_cast<PacketType>(type);
	}

	template<typename Ty>
	inline auto* PacketCast(unsigned char* buffer)
	{
		return reinterpret_cast<Ty*>(buffer);
	}

#pragma pack(push, 1)
	struct RawFloat3
	{
		float x;
		float y;
		float z;
		
		RawFloat3()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
		}

		RawFloat3(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		RawFloat3(DirectX::XMFLOAT3 data)
		{
			this->x = data.x;
			this->y = data.y;
			this->z = data.z;
		}

		operator DirectX::XMFLOAT3()
		{
			return DirectX::XMFLOAT3(x, y, z);
		}
	};

	struct RawFloat4
	{
		float x;
		float y;
		float z;
		float w;

		RawFloat4()
		{
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 0;
		}


		RawFloat4(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		RawFloat4(DirectX::XMFLOAT4 data)
		{
			this->x = data.x;
			this->y = data.y;
			this->z = data.z;
			this->w = data.w;
		}

		operator DirectX::XMFLOAT4()
		{
			return DirectX::XMFLOAT4(x, y, z, w);
		}
	};

	struct PacketHeader
	{
		unsigned char size;
		unsigned char type;
		KG::Server::NET_OBJECT_ID objectId;
	};
	
	
	struct SC_PLAYER_INIT
	{
		DEFAULT_PACKET_HEADER(SC_PLAYER_INIT);
		KG::Server::NET_OBJECT_ID playerObjectId;
		RawFloat3 position;
		RawFloat4 rotation;
	};


	struct INPUTS
	{
		unsigned char stateW;
		unsigned char stateA;
		unsigned char stateS;
		unsigned char stateD;
		unsigned char stateShift;
	};

	struct SC_PLAYER_DATA // 플레이어, 적 관련 주기적으로 송신
	{
		DEFAULT_PACKET_HEADER(SC_PLAYER_DATA);
		RawFloat3 position;
		RawFloat4 rotation;
		INPUTS inputs;
        float playerHp;
		float forwardValue;
		float rightValue;
	};

	struct SC_ADD_OBJECT
	{
		DEFAULT_PACKET_HEADER(SC_ADD_OBJECT);
		KG::Utill::hashType presetId;
		KG::Utill::hashType parentTag;
		KG::Utill::hashType objectTag;
		KG::Server::NET_OBJECT_ID newObjectId;
		RawFloat3 position;
		RawFloat4 rotation;
	};

	struct SC_SCALE_OBJECT
	{
		DEFAULT_PACKET_HEADER(SC_SCALE_OBJECT);
		RawFloat3 scale;
	};

	struct SC_REMOVE_OBJECT
	{
		DEFAULT_PACKET_HEADER(SC_REMOVE_OBJECT);
	};

	// 1. 팀 캐릭터 지우라고 명령 (서버에서 지우면서 패킷 전송)
	// 2. 플레이어 캐릭터 지우라고 명령 (서버에서 팀 캐릭터 지울때 같이 전송)

	struct SC_REMOVE_PLAYER
	{
		DEFAULT_PACKET_HEADER(SC_REMOVE_PLAYER);
	};

	struct SC_MOVE_OBJECT
	{
		DEFAULT_PACKET_HEADER(SC_MOVE_OBJECT);
		RawFloat3 position;
		RawFloat4 rotation;
		// KG::Utill::hashType parentTag;
	};

	struct SC_SYNC_ANIMATION		// test
	{
		DEFAULT_PACKET_HEADER(SC_SYNC_ANIMATION);
		float timer;
	};

	struct SC_CHANGE_ANIMATION
	{
		DEFAULT_PACKET_HEADER(SC_CHANGE_ANIMATION);
		KG::Utill::hashType animId;
		UINT animIndex;
		UINT nextState;
		float blendingTime;
		int repeat;
	};

	struct SC_ADD_PLAYER
	{
		DEFAULT_PACKET_HEADER(SC_ADD_PLAYER);
		KG::Server::NET_OBJECT_ID playerObjectId;
		RawFloat3 position;
		RawFloat4 rotation;
	};
	struct SC_PLAYER_SYNC
	{
		DEFAULT_PACKET_HEADER(SC_PLAYER_SYNC);
		RawFloat4 rotation;
	};

    struct SC_ENEMY_HP
    {
        DEFAULT_PACKET_HEADER(SC_ENEMY_HP);
        float percentage;
    };

	struct SC_LOBBY_DATA
	{
		DEFAULT_PACKET_HEADER(SC_LOBBY_DATA);
		char playerinfo[4];
	};
	
	struct SC_LOGIN_OK
	{
		DEFAULT_PACKET_HEADER(SC_LOGIN_OK);
		char lobbyid;
	};

	struct SC_LOBBY_FULL
	{
		DEFAULT_PACKET_HEADER(SC_LOBBY_FULL);
	};
	
	struct SC_GAME_START
	{
		DEFAULT_PACKET_HEADER(SC_GAME_START);
	};

	struct SC_GAME_END
	{
		DEFAULT_PACKET_HEADER(SC_GAME_END);
	};

	struct CS_REQ_LOGIN
	{
		DEFAULT_PACKET_HEADER(CS_REQ_LOGIN);
	};

	struct CS_INPUT
	{
		DEFAULT_PACKET_HEADER(CS_INPUT);
		unsigned char stateW;
		unsigned char stateA;
		unsigned char stateS;
		unsigned char stateD;
		unsigned char stateShift;
		RawFloat4 rotation;
	};

	struct CS_FIRE
	{
		DEFAULT_PACKET_HEADER(CS_FIRE);
		RawFloat3 origin;
		RawFloat3 direction;
		float distance;
	};

	struct CS_RELOAD
	{
		DEFAULT_PACKET_HEADER(CS_RELOAD);
	};


	struct CS_LOBBY_CHANGE
	{
		DEFAULT_PACKET_HEADER(CS_LOBBY_CHANGE);
		char id; // 플레이어 번호
		char state; // 변경 상태
	};

	struct CS_SELECT_MAP
	{
		DEFAULT_PACKET_HEADER(CS_SELECT_MAP);
		int mapnum; // 맵 번호
	};

	

#pragma pack(pop)

};
