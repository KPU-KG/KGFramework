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
	class DLL INetwork
	{
	public:
		virtual void Initialize() = 0;
		virtual void SetAddress(DWORD address) = 0;
		virtual void Connect() = 0;
		virtual void Close() = 0;
		virtual void SetScene(KG::Core::Scene* scene) = 0;


		virtual void TryRecv() = 0;
		virtual void SendPacket(unsigned char* data) = 0;

		template <typename PacketType>
		void SendTypedPacket(const PacketType& packet)
		{
			this->SendPacket(static_cast<unsigned char*>(&packet));
		}

		virtual void GetNewPlayerNetworkController() = 0;
		virtual void PostComponentProvider(KG::Component::ComponentProvider& provider) = 0;

		virtual void DrawImGUI() = 0;

		virtual bool IsConnected() const = 0;

		bool SetGUIContext(ImGuiContext* context)
		{
			ImGui::SetCurrentContext(context);
			return true;
		};
	};

	DLL KG::Server::INetwork* GetNetwork();
};
