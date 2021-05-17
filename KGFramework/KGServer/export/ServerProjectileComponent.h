#pragma once
#include "ServerBaseComponent.h"


namespace KG::Component
{
	class DynamicRigidComponent;
	// class IAnimationControllerComponent;
	
	// enum class ProjectileType {
	// 	BULLET = 0
	// };

	class DLL SProjectileComponent : public SBaseComponent
	{
	private:
		DynamicRigidComponent* rigid = nullptr;
		TransformComponent* transform = nullptr;

		DirectX::XMFLOAT3 direction;
		float speed;
		float damage;

		bool isDelete = false;

		float sendInterval = 1.f / 30.f;
		float sendTimer = 0.f;
		// ProjectileType type;


		// direction
		// speed
		// damage?
		// type
		// team?

	public:
		SProjectileComponent();
		void Initialize(DirectX::XMFLOAT3 origin, DirectX::XMFLOAT3 direction, float speed, float damage = 1);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override;
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		// virtual void Destroy() override;
		bool IsDelete() const;
	};

	REGISTER_COMPONENT_ID(SProjectileComponent);
}