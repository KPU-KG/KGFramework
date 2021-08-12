#pragma once
#include "ServerBaseComponent.h"
#include "ServerEnemyUnitComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <unordered_set>

namespace KG::Component
{

	class SEnemyMechComponent;
	//////////////////////////////////////////////////////////////////////////////
	// Enemy Actions
	//////////////////////////////////////////////////////////////////////////////
	struct MechIdleAction : public Action {
		MechIdleAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechSetGoalAction : public Action {
		MechSetGoalAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechSetTargetAction : public Action {
		MechSetTargetAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechMoveAction : public Action {
		MechMoveAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechRotateAction : public Action {
		MechRotateAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechAttackAction : public Action {
		MechAttackAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechCheckAttackableAction : public Action {
		MechCheckAttackableAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct MechCheckRootAction : public Action {
		MechCheckRootAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct MechWanderState : public State {
		const static size_t WANDER_ACTION_COUNT = 4;
		std::array<Action*, WANDER_ACTION_COUNT> action;
		const static size_t WANDER_ACTION_IDLE = 0;
		const static size_t WANDER_ACTION_SET_GOAL = 1;
		const static size_t WANDER_ACTION_ROTATE = 2;
		const static size_t WANDER_ACTION_MOVE = 3;

		int curAction = WANDER_ACTION_IDLE;
		MechWanderState(SEnemyUnitComponent* comp) : State(comp) {}
		~MechWanderState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	struct MechTraceState : public State {
		const static size_t TRACE_ACTION_COUNT = 6;
		std::array<Action*, TRACE_ACTION_COUNT> action;

		const static size_t TRACE_ACTION_SET_TARGET_ROTATION = 0;
		const static size_t TRACE_ACTION_ROTATE = 1;
		const static size_t TRACE_ACTION_ATTACK = 2;
		const static size_t TRACE_ACTION_CHECK_ATTACKABLE = 3;
		const static size_t TRACE_ACTION_CHECK_ROOT = 4;
		const static size_t TRACE_ACTION_MOVE = 5;

		int curAction = TRACE_ACTION_CHECK_ATTACKABLE;

		MechTraceState(SEnemyUnitComponent* comp) : State(comp) {}
		~MechTraceState();

		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct MechStateManager {
		SEnemyMechComponent* enemyComp;

		const static size_t STATE_WANDER = 0;
		const static size_t STATE_TRACE = 1;

		const static size_t STATE_COUNT = 2;
		int curState = 0;
		std::array<State*, STATE_COUNT> state;
		MechStateManager(SEnemyMechComponent* comp);
		~MechStateManager();


		void Init();
		void SetState();
		void Execute(float elapsedTime);
		int GetCurState() const;
	};


	static struct MechAnimIndex {
		const static UINT dead = 1U;
		const static UINT shotBigCannon = 6U;
		const static UINT shotSmallCanon = 8U;
		const static UINT walk = 11U;
		const static UINT walkInPlace = 12U;
	};

	class DLL SEnemyMechComponent : public SEnemyUnitComponent
	{
	protected:
		static constexpr int maxHp = 10;

		bool noObstacleInAttack = false;
		bool isMovableInTrace = false;
		bool isAttackRotation = false;

		// DirectX::XMFLOAT3							goal = { 0,0,0 };
		// float traceStateSpeed = 3;

		float wanderRotateSpeed = 1;
		float wanderMoveSpeed = 3;

		float traceRotateSpeed = 3;
		float traceMoveSpeed = 6;

		float										goalDistance = 0;
		float										moveDistance = 0;

		// float										arriveTime = 0;
		// float										moveTime = 0;
		// int checkTime = 0;
		DirectX::XMFLOAT2							angle;

		float										traceRange = 70;
		float										attackRange = 50;

		float										attackInterval = 2;
		float										attackTimer = 0;

		MechStateManager*							stateManager;

	public:
		void SetMoveTime(float t) { moveTime = t; }
		void MoveTimer(float elapsedTime) { moveTime += elapsedTime; }
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		bool SetGoal();
		bool Rotate(float elapsedTime);
		bool Move(float elapsedTime);
		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const float goalDistance) const;
		bool AttackTarget(float elapsedTime);
		bool Idle(float elapsedTime);

		virtual void Attack(SGameManagerComponent* gameManager) override;
		SEnemyMechComponent();

		virtual void OnCreate(KG::Core::GameObject* obj) override;
		virtual void Update(float elapsedTime) override;
		virtual void OnDestroy() override
		{
			IComponent::OnDestroy();
		}
		virtual bool OnDrawGUI();

		virtual void HitBullet() override;

		bool SetTarget();
		bool SetAttackRotation();
		bool IsTargetInRange() const;
		bool CheckAttackable();
		bool NoObstacleInAttack() const;
		bool CheckRoot();

		bool IsMobableInTrace() const;
		// bool IsPathFinding() const;
		bool IsAttackRotation() const;
		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyMechComponent);
}