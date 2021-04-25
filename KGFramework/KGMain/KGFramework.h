#pragma once
#define NOMINMAX
#include <Windows.h>
#include "Setting.h"
#include "GameTimer.h"
#include "KGRenderer.h"
#include "IPhysicsScene.h"
#include "InputManager.h"
#include "IKGServer.h"
#include "IKGNetwork.h"
#include "ComponentProvider.h"
#include "Scene.h"
#include <memory>
namespace KG
{
	struct EngineDesc
	{
		HWND hWnd;
		HINSTANCE hInst;
	};

	struct Systems;

	class GameFramework
	{
		EngineDesc engineDesc;
		Setting setting;
		GameTimer timer;
		std::unique_ptr<Systems> system;
		std::unique_ptr<KG::Renderer::IKGRenderer> renderer;
		KG::Physics::IPhysicsScene* physics;
		std::unique_ptr<KG::Input::InputManager> input;
		std::unique_ptr<KG::Server::INetwork> networkClient;
		std::unique_ptr<KG::Server::IServer> networkServer;
		std::unique_ptr<KG::Core::Scene> scene;
		KG::Component::ComponentProvider componentProvider;
		ImGuiContext* guiContext = nullptr;
	public:
		GameFramework();
		~GameFramework();
		GameFramework(const GameFramework& rhs);
		GameFramework& operator=(const GameFramework& rhs);
		GameFramework(GameFramework&& rhs);
		GameFramework& operator=(GameFramework&& rhs);

		bool Initialize(const EngineDesc& engineDesc, const Setting& setting);
		void PostSceneFunction();
		void PostNetworkFunction();
		void PostServerFunction();
		int WinProcHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		void UIPreRender();
		void UIRender();
		void OnProcess();
		void ServerProcess();
		void ServerUpdate(float elapsedTime);
		void ServerProcessEnd();
		void OnClose();
	private:
		std::wstring windowText = L"KG Framework : FPS : ##########";
		void UpdateWindowText();
	};
};