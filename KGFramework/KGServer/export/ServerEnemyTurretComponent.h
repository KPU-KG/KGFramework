#pragma once
#include "ServerBaseComponent.h"
#include "ServerEnemyUnitComponent.h"
#include "Debug.h"
#include <vector>
#include <unordered_set>
#include <array>

namespace KG::Component
{
	
	class SEnemyTurretComponent;
	//////////////////////////////////////////////////////////////////////////////
	// Enemy Actions
	//////////////////////////////////////////////////////////////////////////////
	struct TurretIdleAction : public Action {
		TurretIdleAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct TurretSetTargetAction : public Action {
		TurretSetTargetAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct TurretRotateAction : public Action {
		TurretRotateAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct TurretAttackAction : public Action {
		TurretAttackAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct TurretAttackState : public State {
		const static size_t ATTACK_ACTION_COUNT = 4;
		std::array<Action*, ATTACK_ACTION_COUNT> action;
		const static size_t ATTACK_ACTION_IDLE = 0;
		const static size_t ATTACK_ACTION_SET_TARGET = 1;
		const static size_t ATTACK_ACTION_ROTATE = 2;
		const static size_t ATTACK_ACTION_ATTACK = 3;

		int curAction = ATTACK_ACTION_IDLE;
		TurretAttackState(SEnemyUnitComponent* comp) : State(comp) {}
		~TurretAttackState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct TurretStateManager {
		SEnemyTurretComponent* enemyComp;

		const static size_t STATE_ATTACK = 0;
		const static size_t STATE_COUNT = 1;

		int curState = STATE_ATTACK;
		std::array<State*, STATE_COUNT> state;
		TurretStateManager(SEnemyTurretComponent* comp) : enemyComp(comp) {};
		~TurretStateManager();


		void Init();
		void SetState();
		void Execute(float elapsedTime);
		int GetCurState() const;
	};

	class DLL SEnemyTurretComponent : public SEnemyUnitComponent
	{
	protected:
		static constexpr int maxHp = 10;
		static constexpr int attackCount = 3;

		int curAttackCount = 0;
		float rotateSpeed = 1;
		float attackRange = 70;
		
		bool inAttack = false;
		float										attackInterval = 1;
		float										attackTimer = 0;
		float tempAttackTimer = 0;
		TransformComponent* gunTransform = nullptr;
		TurretStateManager* stateManager;

	public:
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		// 1. 범위 안에 들어오면 레이캐스팅으로 쏠 수 있는 한 놈 설정
		// 2. 회전
		// 3. 공격

		bool Idle(float elapsedTime);
		bool SetTarget();
		bool Rotate(float elapsedTime);
		bool AttackTarget(float elapsedTime);
		float GetDistance2FromPos(DirectX::XMFLOAT3 pos) const;
		bool IsInAttack() const;

		virtual void Attack(SGameManagerComponent* gameManager) override;
		SEnemyTurretComponent();

		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}

		virtual bool OnDrawGUI();
		virtual void HitBullet() override;
		virtual void Awake() override;

		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyTurretComponent);
}