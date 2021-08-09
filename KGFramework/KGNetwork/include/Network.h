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
#include "ClientProjectileComponent.h"
#include "ClientCubeAreaRedComponent.h"
#include "ClientSystems.h"

namespace KG::Server
{
	struct NetworkBuffer
	{
        static constexpr UINT NETWORK_MAX_BUFFER = MAX_BUFFER * MAX_BUFFER * 2;
		WSABUF wsaBuffer;
		unsigned char buffer[NETWORK_MAX_BUFFER];
	};

	class Network : public INetwork
	{
		SOCKADDR_IN serverAddr;
		SOCKET clientSocket;
		int prevRecvSize = 0;
		NetworkBuffer networkBuffer;

		KG::Core::Scene* scene;
		KG::System::CGameManagerComponentSystem cGameManagerSystem;
		KG::System::CPlayerControllerComponentSystem cPlayerSystem;
		KG::System::CCharacterComponentSystem cCharacterSystem;
		KG::System::CEnemyControllerComponentSystem cEnemyControllerSystem;
		KG::System::CProjectileComponentSystem cProjectileSystem;
		KG::System::CCubeAreaRedComponentSystem cCubeAreaRedSystem;

		std::unordered_map<KG::Server::NET_OBJECT_ID, KG::Component::CBaseComponent*> networkObjects;

		void ProcessPacket(unsigned char* buffer);
	public:
		virtual void Initialize() override;
		virtual void SetAddress(DWORD address) override;
        virtual void SetAddress(const std::string& address) override;
		virtual void Connect() override;
        virtual bool TryConnect() override;
		virtual void Close() override;
		virtual void TryRecv() override;
		virtual void SendPacket(void* data);
        virtual void Login() override;

		virtual void SetNetworkObject(KG::Server::NET_OBJECT_ID id, KG::Component::CBaseComponent* obj);

		virtual KG::Component::CGameManagerComponent* GetNewGameManagerComponent() override;
		virtual KG::Component::CPlayerControllerComponent* GetNewPlayerControllerComponent() override;
		virtual KG::Component::CCharacterComponent* GetNewCharacterComponent() override;
		virtual KG::Component::CEnemyControllerComponent* GetNewEnemyControllerOomponent() override;
		virtual KG::Component::CProjectileComponent* GetNewProjectileComponent() override;
		virtual KG::Component::CCubeAreaRedComponent* GetNewCubeAreaRedComponent() override;

		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) override;

		virtual void DrawImGUI() override;
		virtual bool IsConnected() const override;
		virtual void SetScene(KG::Core::Scene* scene) override;



		// INetwork을(를) 통해 상속됨
		virtual void Update(float elapsedTime) override;


		// INetwork을(를) 통해 상속됨
		virtual void SetInputManager(KG::Input::InputManager* manager) override;





    };
};
