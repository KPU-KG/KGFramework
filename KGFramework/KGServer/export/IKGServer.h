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

namespace KG::Component
{
	class SGameManagerComponent;
	class SPlayerComponent;
};

namespace KG::Server
{
	class DLL IServer
	{
	public:
		virtual void Initialize() = 0;

		virtual void Start() = 0;
		virtual void Close() = 0;

		virtual void LockWorld() = 0;
		virtual void UnlockWorld() = 0;

		virtual KG::Component::SGameManagerComponent* GetNewGameManagerComponent() = 0;
		virtual KG::Component::SPlayerComponent* GetNewPlayerComponent() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;
		virtual void DrawImGUI() = 0;
		virtual bool isStarted() const = 0;
		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		};
	};

	DLL IServer* GetServer();
};
