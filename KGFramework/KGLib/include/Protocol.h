#pragma once
#define DEFAULT_PACKET_HEADER( x ) PacketHeader header = PacketHeader{sizeof(x), PacketID(KG::Packet::PacketType::x), KG::Server::NULL_NET_OBJECT_ID }

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
	constexpr NET_OBJECT_ID NULL_NET_OBJECT_ID = -343434;

};
namespace KG::Packet
{
	enum class PacketType : unsigned char
	{
		None = 0,
		PacketHeader = 1,

		SC_LOGIN_OK = 100, // 초기버전 미사용
		SC_PLAYER_INFO,
		SC_ADD_OBJECT,
		SC_REMOVE_OBJECT, // 초기버전 미사용
		SC_FIRE, // 초기버전 미사용

		CS_LOGIN = 200, // 초기버전 미사용
		CS_INPUT, // 사용
		CS_FIRE // 초기버전 미사용
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
	
	//struct SC_LOGIN_OK
	//{
	//	DEFAULT_PACKET_HEADER(SC_LOGIN_OK);
	//};

	struct SC_PLAYER_INFO
	{
		DEFAULT_PACKET_HEADER(SC_PLAYER_INFO);
		int playerId;
		RawFloat3 position;
		RawFloat3 rotation;
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

	struct CS_INPUT
	{
		DEFAULT_PACKET_HEADER(CS_INPUT);
		unsigned char stateW;
		unsigned char stateA;
		unsigned char stateS;
		unsigned char stateD;
		unsigned char stateShift;
	};

#pragma pack(pop)

};
