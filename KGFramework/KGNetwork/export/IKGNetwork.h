#pragma once

#include "ISystem.h"
#include "IComponent.h"
#include "Protocol.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

namespace KG::Input
{
	class InputManager;
}

namespace KG::Component
{
	class CBaseComponent;
	class CGameManagerComponent;
	class CPlayerControllerComponent;
	class CCharacterComponent;
	class CEnemyControllerComponent;
	class CProjectileComponent;
	class CCubeAreaRedComponent;
	class CLobbyComponent;
}

namespace KG::Server
{
	class DLL INetwork
	{
	public:
		virtual void Initialize() = 0;
        virtual void SetAddress(DWORD address) = 0;
        virtual void SetAddress(const std::string& address) = 0;
        virtual void Connect() = 0;
        virtual bool TryConnect() = 0;
        virtual void Login() = 0;
		virtual void Close() = 0;
		virtual void SetScene(KG::Core::Scene* scene) = 0;
		virtual void TryRecv() = 0;
		virtual void Update(float elapsedTime) = 0;
		virtual KG::Component::CGameManagerComponent* GetNewGameManagerComponent() = 0;
		virtual KG::Component::CPlayerControllerComponent* GetNewPlayerControllerComponent() = 0;
		virtual KG::Component::CCharacterComponent* GetNewCharacterComponent() = 0;
		virtual KG::Component::CEnemyControllerComponent* GetNewEnemyControllerOomponent() = 0;
		virtual KG::Component::CProjectileComponent* GetNewProjectileComponent() = 0;
		virtual KG::Component::CCubeAreaRedComponent* GetNewCubeAreaRedComponent() = 0;
		virtual KG::Component::CLobbyComponent* GetNewLobbyComponent() = 0;

		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;

		virtual void DrawImGUI() = 0;

		virtual bool IsConnected() const = 0;

		virtual void SetInputManager(KG::Input::InputManager* manager) = 0;

		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		};
	};

	DLL KG::Server::INetwork* GetNetwork();
};
