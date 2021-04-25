#pragma once
#include <WS2tcpip.h>
#include <windows.h>
#include <unordered_map>
#include "IKGNetwork.h"
#include "Protocol.h"
#include "ISystem.h"
#include "ClientGameManagerComponent.h"
#include "ClientCharacterComponent.h"
#include "ClientPlayerControllerComponent.h"
#include "ClientEnemyControllerComponent.h"

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

		KG::Core::Scene* scene;
		KG::Component::CGameManagerComponentSystem cGameManagerSystem;
		KG::Component::CPlayerControllerComponentSystem cPlayerSystem;
		KG::Component::CCharacterComponentSystem cCharacterSystem;
		KG::Component::CEnemyControllerComponentSystem cEnemyControllerSystem;


		std::unordered_map<KG::Server::NET_OBJECT_ID, KG::Component::CBaseComponent*> networkObjects;

		void ProcessPacket(unsigned char* buffer);
	public:
		virtual void Initialize() override;
		virtual void SetAddress(DWORD address) override;
		virtual void Connect() override;
		virtual void Close() override;
		virtual void TryRecv() override;
		virtual void SendPacket(void* data);

		virtual void SetNetworkObject(KG::Server::NET_OBJECT_ID id, KG::Component::CBaseComponent* obj);

		virtual KG::Component::CGameManagerComponent* GetNewGameManagerComponent() override;
		virtual KG::Component::CPlayerControllerComponent* GetNewPlayerControllerComponent() override;
		virtual KG::Component::CCharacterComponent* GetNewCharacterComponent() override;
		virtual KG::Component::CEnemyControllerComponent* GetNewEnemyControllerOomponent() override;

		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

		virtual void DrawImGUI() override;
		virtual bool IsConnected() const override;
		virtual void SetScene(KG::Core::Scene* scene) override;




		// INetwork��(��) ���� ��ӵ�
		virtual void Update(float elapsedTime) override;

	};
};