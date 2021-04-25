#pragma once
#include "pch.h"
#include "IComponent.h"
#include "ISystem.h"
#include "ClientBaseComponent.h"
#include "Debug.h"
#include <vector>

namespace KG::Component
{
	// enum class EnemyAction {
	// 	eIDLE = 0,
	// 	eSETGOAL,
	// 	eROTATE,
	// 	eMOVE,
	// 	eATTACK,
	// 	eATTACKED
	// };
	// 
	// enum class EnemyState {
	// 	eWANDER = 0,			
	// 	eTRACE,
	// 	eRUNAWAY
	// };

	// 일단 애니메이션 정보는 후에 보내기
	// static struct MechAnimIndex {
	// 	const static UINT shotSmallCanon = 8U;
	// 	const static UINT walk = 11U;
	// 	const static UINT walkInPlace = 12U;
	// };

	class DynamicRigidComponent;
	class AnimationControllerComponent;

	class DLL CEnemyControllerComponent : public CBaseComponent
	{
	private:
		DynamicRigidComponent* rigid = nullptr;
		TransformComponent* transform = nullptr;
		AnimationControllerComponent* anim = nullptr;

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