#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "Debug.h"

namespace KG::Physics {
	class IPhysicsScene;
}

static enum LobbyState
{
	Empty,
	Wait,
	Ready,
};

namespace KG::Component
{
	class DLL SLobbyComponent : public SBaseComponent
	{
	private:
		// �÷��̾� ���� ����
		char PlayerInfo[PLAYERNUM] = { LobbyState::Empty, };
		// *
		// ���� �������� �� Wait�� ����
	public:
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void Destroy() override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnDrawGUI();
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender); // sendpacket
	};
	REGISTER_COMPONENT_ID(SLobbyComponent);
}

