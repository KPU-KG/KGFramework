#pragma once
#include "ClientBaseComponent.h"

namespace KG::Component
{
	class DLL CCubeAreaRedComponent : public CBaseComponent
	{
	private:
		TransformComponent* transform = nullptr;

	public:
		CCubeAreaRedComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
		virtual bool OnDrawGUI();

	};

	REGISTER_COMPONENT_ID(CCubeAreaRedComponent);


}