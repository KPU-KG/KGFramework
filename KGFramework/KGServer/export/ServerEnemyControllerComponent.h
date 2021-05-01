#pragma once
#include "IComponent.h"
#include "ISystem.h"
#include "ServerBaseComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>

namespace KG::Component
{
	// 1. 주어진 위치에서 범위 설정 - 이거 GUI 필요할까?
	// 2. 범위 내에서 돌아다니기 - 이건 노드 설치하면 A*나 길찾기 알고리즘 적용해야 할듯

	// action
	// 대기
	// 이동
	// 공격
	// 피격

	// state
	// 배회
	// 추적 (플레이어 발견)
	// 도망 (?)
	enum class EnemyAction {
		eIDLE = 0,
		eSETGOAL,
		eROTATE,
		eMOVE,
		eATTACK,
		eATTACKED
	};

	enum class EnemyState {
		eWANDER = 0,			// 타겟 위치 정하기 -> 이동하기 -> 잠시 대기 -> 타겟 위치 정하기
		eTRACE,					
		eRUNAWAY
	};

	static struct MechAnimIndex {
		const static UINT dead = 1U;
		const static UINT shotSmallCanon = 8U;
		const static UINT walk = 11U;
		const static UINT walkInPlace = 12U;
	};

	class DynamicRigidComponent;
	class AnimationControllerComponent;


	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		AnimationControllerComponent*				anim = nullptr;

		EnemyAction				action = EnemyAction::eSETGOAL;
		EnemyState				state = EnemyState::eWANDER;

		DirectX::XMFLOAT3		center = { 0,0,0 };			// onCreate에서 정해줌
		float					range = 10;
		DirectX::XMFLOAT3		direction = { 0,0,0 };		// 일단 z값은 고려하지 않을 예정이나 비행 몹에는 쓸지도..?
		DirectX::XMFLOAT3		goal = { 0,0,0 };
		float					speed = 3;
		float					idleInterval = 3;
		float					idleTimer = 0;
		float					rotateInterval = 2;
		float					rotateTimer = 0;
		
		KG::Component::RaycastCallbackFunc raycastCallback = nullptr;

		int						hp = 10;

		DirectX::XMFLOAT2		angle;
		void UpdateState();
		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal();
		bool Idle(float elapsedTime);
	public:
		SEnemyControllerComponent();
		void SetCenter(DirectX::XMFLOAT3 center) {
			this->center = center;
		}
		void SetSpeed(float speed) {
			this->speed = speed;
		}
		void SetIdleInterval(float interval) {
			this->idleInterval = interval;
		}
		void SetRotateInterval(float interval) {
			this->rotateInterval = interval;
		}
		void SetWanderRange(float range) {
			this->range = range;
		}
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();
		void SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback) {
			this->raycastCallback = callback;
		}
		void HitBullet() {
			this->hp -= 1;
		}
	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);


}