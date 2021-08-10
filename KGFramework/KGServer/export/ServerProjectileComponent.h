#pragma once
#include "ServerBaseComponent.h"


namespace KG::Component
{
	class DynamicRigidComponent;

	class DLL SProjectileComponent : public SBaseComponent
	{
	private:
		DynamicRigidComponent* rigid = nullptr;
		TransformComponent* transform = nullptr;

		DirectX::XMFLOAT3 direction;
		float speed;
		float damage;
		DirectX::XMFLOAT3 targetPos;

		bool isDelete = false;
		bool isSetTarget = false;

		float sendInterval = 1.f / 30.f;
		float sendTimer = 0.f;
	public:
		SProjectileComponent();
		void Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float speed, float damage = 1);
		void SetTargetPosition(DirectX::XMFLOAT3 pos);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override;
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		bool IsDelete() const;
	};

	class DLL SCrawlerMissileComponent : public SBaseComponent {
	private:
		DynamicRigidComponent* rigid = nullptr;
		TransformComponent* transform = nullptr;

		DirectX::XMFLOAT3 direction;
		DirectX::XMFLOAT3 targetPos;
		float curveHeight;
		DirectX::XMFLOAT3 topPosition;

		float moveInterval;
		float moveTimer;
		bool isCurved;

		float speed;
		float damage = 1;

		bool isDelete = false;

		float sendInterval = 1.f / 30.f;
		float sendTimer = 0.f;
	public:
		SCrawlerMissileComponent();
		void Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 target);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override;
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		bool IsDelete() const;
	};

	REGISTER_COMPONENT_ID(SProjectileComponent);
	REGISTER_COMPONENT_ID(SCrawlerMissileComponent);
}