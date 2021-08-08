#pragma once
#include "pch.h"
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include <vector>

namespace KG::Component
{
	class DynamicRigidComponent;
	// class IAnimationControllerComponent;
	// class IRenderSpriteComponent;

	class DLL CProjectileComponent : public CBaseComponent
	{
	private:
		TransformComponent* transform = nullptr;

	public:
		CProjectileComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
		virtual bool OnDrawGUI();

	};

	REGISTER_COMPONENT_ID(CProjectileComponent);


}