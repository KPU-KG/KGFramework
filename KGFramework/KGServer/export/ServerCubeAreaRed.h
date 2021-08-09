#pragma once
#include "ServerBaseComponent.h"

namespace KG::Component
{
	class DLL SCubeAreaRedComponent : public SBaseComponent
	{
	private:
		TransformComponent* transform = nullptr;
		float fillTimer = 0;
		float fillInterval = 0;
		bool isDelete = false;

		float width;

		float sendInterval = 1.f / 30.f;
		float sendTimer = 0.f;
	public:
		SCubeAreaRedComponent();
		void Initialize(DirectX::XMFLOAT3 center, float width, float interval = 0);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override;
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		bool IsDelete() const;
	};

	REGISTER_COMPONENT_ID(SCubeAreaRedComponent);
}