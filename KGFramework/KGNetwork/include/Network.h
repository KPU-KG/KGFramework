#pragma once
#include <WS2tcpip.h>
#include <windows.h>
#include "IKGNetwork.h"
#include "Protocol.h"
namespace KG::Server
{
	struct NetworkBuffer
	{
		WSABUF wsaBuffer;
		unsigned char buffer[MAX_BUFFER];
	};

	class Network : public INetwork
	{
		SOCKADDR_IN serverAddr;
		SOCKET clientSocket;
		int prevRecvSize = 0;
		NetworkBuffer networkBuffer;
		// INetwork을(를) 통해 상속됨

		void ProcessPacket(unsigned char* buffer);
	public:
		virtual void Initialize() override;
		virtual void SetAddress(DWORD address) override;
		virtual void Connect() override;
		virtual void Close() override;
		virtual void TryRecv() override;
		virtual void SendPacket(unsigned char* data) override;
		virtual void GetNewPlayerNetworkController() override;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;
		virtual void DrawImGUI() override;
		virtual bool IsConnected() const override;
	};
};
