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
    class IAnimationControllerComponent;
    class IRenderSpriteComponent;
	class ISoundComponent;
    class IParticleEmitterComponent;

	class DLL CEnemyControllerComponent : public CBaseComponent
	{
	private:
        TransformComponent* transform = nullptr;
        TransformComponent* pelvis = nullptr;
		TransformComponent* gunTransform = nullptr;
        IAnimationControllerComponent* anim = nullptr;
        IRenderSpriteComponent* hpBar = nullptr;
        IParticleEmitterComponent* particle = nullptr;

        float prevHP = 0.0f;
	public:
		CEnemyControllerComponent();
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type) override;
		virtual bool OnDrawGUI();

	};

	REGISTER_COMPONENT_ID(CEnemyControllerComponent);


}