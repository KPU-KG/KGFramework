#include "pch.h"
#include <WS2tcpip.h>
#include <windows.h>
#include "Network.h"
#include "NetworkUtill.h"
#include "Scene.h"
#include "InputManager.h"


DLL KG::Server::INetwork* KG::Server::GetNetwork()
{
	return new Network();
}

void KG::Server::Network::Initialize()
{
	WSAData wsaData = {};
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	this->cGameManagerSystem.SetNetworkInstance(this);
}

void KG::Server::Network::SetAddress(DWORD address)
{
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_addr.s_addr = (address);
	this->serverAddr.sin_port = htons(SERVER_PORT);
}

void KG::Server::Network::SetAddress(const std::string& address)
{
    DWORD ipWord;
    inet_pton(AF_INET, address.c_str(), &ipWord);
    this->SetAddress(ipWord);
}

void KG::Server::Network::Connect()
{
	clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, 0);
	int ret = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
	std::cout << "Try Connect\n";
	if ( SOCKET_ERROR == ret )
	{
		display_error(L"connect", WSAGetLastError());
	}
	unsigned long noblock = 1;
	ioctlsocket(clientSocket, FIONBIO, &noblock);
}

void KG::Server::Network::Close()
{
	closesocket(clientSocket);
	WSACleanup();
}

void KG::Server::Network::TryRecv()
{
	DWORD recvFlag = 0;
	DWORD dataBytes = 0;

	networkBuffer.wsaBuffer.buf = reinterpret_cast<CHAR*>(networkBuffer.buffer + this->prevRecvSize);
	networkBuffer.wsaBuffer.len = MAX_BUFFER - this->prevRecvSize;

	int ret = WSARecv(this->clientSocket, &this->networkBuffer.wsaBuffer, 1, &dataBytes, &recvFlag, nullptr, nullptr);
	if ( ret == SOCKET_ERROR )
	{
		int error = WSAGetLastError();
		if ( error == WSAEWOULDBLOCK )
		{
			return;
		}
		else
		{
			display_error(L"TryRecv", error);
			return;
		}
	}

	unsigned char* packet_ptr = this->networkBuffer.buffer;
	int num_data = dataBytes + this->prevRecvSize;
	int packetSize = packet_ptr[0];

	while ( num_data != 0 && num_data >= packetSize ) // ´Ù¹ÞÀ½
	{
		ProcessPacket(packet_ptr);
		num_data -= packetSize;
		packet_ptr += packetSize;
		packetSize = packet_ptr[0];
	}
	this->prevRecvSize = num_data;
	if ( num_data != 0 )
	{
		memcpy(this->networkBuffer.buffer, packet_ptr, num_data);
	}
}

void KG::Server::Network::ProcessPacket(unsigned char* buffer)
{
	auto* header = reinterpret_cast<KG::Packet::PacketHeader*>(buffer);
	auto it = this->networkObjects.find(header->objectId);
	if ( it == this->networkObjects.end() )
	{
		std::cout << "Unknown Packet Receiver Object : " << header->objectId << '\n';
	}
	else
	{
		bool processed = it->second->ProcessPacket(buffer, KG::Packet::ToPacketType(header->type));
		if ( !processed )
		{
			std::cout << "Packet Owner Object Not Processd / Object ID : " << header->objectId << " / PacketType : " << header->type << "\n";
		}
	}
	
}

void KG::Server::Network::SendPacket(void* data)
{
	WSABUF sendBuf;
	sendBuf.buf = (char*)data;
	sendBuf.len = static_cast<unsigned char*>(data)[0];
	while ( true )
	{
		DWORD byteSent = 0;
		int ret = WSASend(clientSocket, &sendBuf, 1, &byteSent, NULL, NULL, NULL);

		if ( SOCKET_ERROR == ret )
		{
			int error = WSAGetLastError();
			if ( error == WSAEWOULDBLOCK )
			{
				sendBuf.buf += byteSent;
				sendBuf.len -= byteSent;
				continue;
			}
			else
			{
				display_error(L"send", error);
				return;
			}
		}
		else
		{
			sendBuf.buf += byteSent;
			sendBuf.len -= byteSent;
			if ( sendBuf.len == 0 )
				return;
		}
	}
}

void KG::Server::Network::SetNetworkObject(KG::Server::NET_OBJECT_ID id, KG::Component::CBaseComponent* obj)
{
	this->networkObjects.emplace(id, obj);
}

KG::Component::CGameManagerComponent* KG::Server::Network::GetNewGameManagerComponent()
{
	auto* comp = this->cGameManagerSystem.GetNewComponent();
	comp->SetNetworkInstance(this);
	return comp;
}

KG::Component::CPlayerControllerComponent* KG::Server::Network::GetNewPlayerControllerComponent()
{
	auto* comp = this->cPlayerSystem.GetNewComponent();
	comp->SetNetworkInstance(this);
	return comp;
}

KG::Component::CCharacterComponent* KG::Server::Network::GetNewCharacterComponent()
{
	auto* comp = this->cCharacterSystem.GetNewComponent();
	comp->SetNetworkInstance(this);
	return comp;
}

KG::Component::CEnemyControllerComponent* KG::Server::Network::GetNewEnemyControllerOomponent()
{
	auto* comp = this->cEnemyControllerSystem.GetNewComponent();
	comp->SetNetworkInstance(this);
	return comp;
}

void KG::Server::Network::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
	this->cGameManagerSystem.OnPostProvider(provider);
	this->cEnemyControllerSystem.OnPostProvider(provider);
}

void KG::Server::Network::DrawImGUI()
{
    static char ipBuffer[256];

    ImGui::BulletText("Is Processing Client");
    inet_ntop(AF_INET, &this->serverAddr.sin_addr, ipBuffer, 256);
    ImGui::Text("%s:%d", ipBuffer, ntohs(this->serverAddr.sin_port));
    
}

bool KG::Server::Network::IsConnected() const
{
	return this->clientSocket != NULL;
}

void KG::Server::Network::SetScene(KG::Core::Scene* scene)
{
	this->scene = scene;
}

void KG::Server::Network::SetInputManager(KG::Input::InputManager* manager)
{
	KG::Input::InputManager::SetInputManager(manager);
}

bool KG::Server::Network::TryConnect()
{
    return false;
}

void KG::Server::Network::Login()
{
    auto* ptr = static_cast<KG::Component::CGameManagerComponent*>(this->networkObjects[0]);
    ptr->SendLoginPacket();
}

void KG::Server::Network::Update(float elapsedTime)
{
	this->cGameManagerSystem.OnUpdate(elapsedTime);
	this->cPlayerSystem.OnUpdate(elapsedTime);
	this->cCharacterSystem.OnUpdate(elapsedTime);
}

