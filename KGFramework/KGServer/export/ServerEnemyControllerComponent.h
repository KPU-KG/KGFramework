#pragma once
#include "ServerBaseComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <unordered_set>

constexpr int MAX_NODE = 5;

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
		eATTACK
	};

	enum class EnemyState {
		eWANDER = 0,			// 타겟 위치 정하기 -> 이동하기 -> 잠시 대기 -> 타겟 위치 정하기
		eTRACE,					
		eRUNAWAY
	};


	// Action Queue에 행동들을 넣고 실행
	// state마다 action이 다름
	// 매 update마다 state 변환 조건 검사
	// 해당 스테이트에 맞게 액션 조건 검사

	class SEnemyControllerComponent;

	struct Action {

		// 조건 검사
		virtual bool CheckCondition() = 0;


	};

	static struct MechAnimIndex {
		const static UINT dead = 1U;
		const static UINT shotBigCannon = 6U;
		const static UINT shotSmallCanon = 8U;
		const static UINT walk = 11U;
		const static UINT walkInPlace = 12U;
	};

	class DynamicRigidComponent;
	class IAnimationControllerComponent;
	class SGameManagerComponent;


	class DLL SEnemyControllerComponent : public SBaseComponent
	{
	private:
		std::unordered_set<KG::Server::NET_OBJECT_ID> playerId;
		KG::Component::SBaseComponent* target = nullptr;

		DynamicRigidComponent*						rigid = nullptr;
		TransformComponent*							transform = nullptr;
		IAnimationControllerComponent*				anim = nullptr;

		EnemyAction									action = EnemyAction::eIDLE;
		EnemyState									state = EnemyState::eWANDER;

		DirectX::XMFLOAT3							center = { 0,0,0 };			// onCreate에서 정해줌
		float										range = 10;
		DirectX::XMFLOAT3							direction = { 0,0,0 };		// 일단 y값은 고려하지 않을 예정이나 비행 몹에는 쓸지도..?
		DirectX::XMFLOAT3							goal = { 0,0,0 };
		float										speed = 3;

		float										idleInterval = 3;
		float										idleTimer = 0;

		float										rotateInterval = 2;
		float										rotateTimer = 0;
		float										rotateAttackInterval = 0.5;

		float										distance = 0;
		float										arriveTime = 0;
		float										moveTime = 0;

		bool										isDead = false;
		float										destroyInterval = 3.f;
		float										destroyTimer = 0.f;

		float										sendTimer = 0.0f;
		float										sendInterval = 1.f / 60.f;

		bool										changedAnimation = false;

		KG::Component::RaycastCallbackFunc			raycastCallback = nullptr;

        static constexpr int maxHp = 10;
        int											hp = maxHp;

		DirectX::XMFLOAT2							angle;

		DirectX::XMFLOAT3							node[MAX_NODE]{ DirectX::XMFLOAT3(0,0,0), };
		int											nodeCount = 0;
		bool										randomCircuit;
		int											currentNode = 0;

		// 일단은 길찾기는 빼고 범위 내로 들어오면 타겟 플레이어를 향해 회전, 가능하면 공격까지
		float										traceRange = 10;				

		float										attackInterval = 2;
		float										attackTimer = 0;
		bool										isInAttackDelay = false;
		bool										isAttackable = false;

		void UpdateState();
		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal();
		bool Idle(float elapsedTime);
		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const float distance) const;
		bool AttackTarget(float elapsedTime);
		// 공격 패킷을 어떻게 보내야 할까
	public:
		bool IsAttackable() const;
		void PostAttack();
		void Attack(SGameManagerComponent* gameManager);
		SEnemyControllerComponent();
		void SetCenter(DirectX::XMFLOAT3 center);
		void SetSpeed(float speed);
		void SetIdleInterval(float interval);
		void SetRotateInterval(float interval);
		void SetWanderRange(float range);
		void SetPosition(DirectX::XMFLOAT3 position);
		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnProcessPacket(unsigned char* packet, KG::Packet::PacketType type, KG::Server::SESSION_ID sender) override;
		virtual bool OnDrawGUI();
		virtual void OnDataLoad(tinyxml2::XMLElement* objectElement) override;
		virtual void OnDataSave(tinyxml2::XMLElement* objectElement) override;
		void SetRaycastCallback(KG::Component::RaycastCallbackFunc&& callback);
		void HitBullet();
		bool IsDead() const;
		bool IsDelete() const;
		bool SetTarget();
		bool IsTargetInRange() const;
		void RegisterPlayerId(KG::Server::NET_OBJECT_ID id);
		void DeregisterPlayerId(KG::Server::NET_OBJECT_ID id);
		KG::Server::NET_OBJECT_ID GetNetId() const { return this->networkObjectId; }
		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyControllerComponent);
}