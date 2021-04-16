#pragma once

#include "ISystem.h"
#include "IComponent.h"

#define EXTERNC extern "C"
#ifdef EXPORTS
#define DLL __declspec(dllexport)
#else
#define DLL __declspec(dllimport)
#endif

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

		virtual void GetNewPlayerNetworkController() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;

		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		};
	};

	DLL IServer* GetServer();
};
