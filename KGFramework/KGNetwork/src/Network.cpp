#include "pch.h"
#include <WS2tcpip.h>
#include <windows.h>
#include "Network.h"
#include "NetworkUtill.h"


DLL KG::Server::INetwork* KG::Server::GetNetwork()
{
	return new Network();
}

void KG::Server::Network::Initialize()
{
	WSAData wsaData = {};
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void KG::Server::Network::SetAddress(DWORD address)
{
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_addr.s_addr = (address);
	this->serverAddr.sin_port = htons(SERVER_PORT);
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
	//switch ( buffer[1] ) // Packet Type
	//{
	//	case P_SC_INIT:
	//		this->ProcessInitPacket(reinterpret_cast<packet_sc_init*>(buffer));
	//		break;
	//	case P_SC_ENTER:
	//		this->ProcessEnterPacket(reinterpret_cast<packet_sc_enter*>(buffer));
	//		break;
	//	case P_SC_LEAVE:
	//		this->ProcessLeavePacket(reinterpret_cast<packet_sc_leave*>(buffer));
	//		break;
	//	case P_SC_MOVE:
	//		this->ProcessMovePacket(reinterpret_cast<packet_sc_move*>(buffer));
	//		break;
	//	default:
	//		assert(false);
	//		break;
	//}
}

void KG::Server::Network::SendPacket(unsigned char* data)
{
	WSABUF sendBuf;
	sendBuf.buf = (char*)data;
	sendBuf.len = data[0];
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

void KG::Server::Network::GetNewPlayerNetworkController()
{
}

void KG::Server::Network::PostComponentProvider(KG::Component::ComponentProvider& provider)
{
}

void KG::Server::Network::DrawImGUI()
{
	static char ipBuffer[256] = "127.0.0.1";
	if ( ImGui::CollapsingHeader("Network Client", ImGuiTreeNodeFlags_DefaultOpen) )
	{
		if ( !this->IsConnected() )
		{
			ImGui::InputText("ipaddress", ipBuffer, 256);
			if ( ImGui::Button("Connect") )
			{
				DWORD ipWord;
				inet_pton(AF_INET, ipBuffer, &ipWord);
				this->SetAddress(ipWord);
				this->Connect();
			}
		}
	}
}

bool KG::Server::Network::IsConnected() const
{
	return this->clientSocket != NULL;
}
