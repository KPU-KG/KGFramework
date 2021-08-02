#pragma once
#include "ServerBaseComponent.h"
#include "ServerEnemyUnitComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <array>
#include <unordered_set>

namespace KG::Component
{
	//////////////////////////////////////////////////////////////////////////////
	// Enemy Actions
	//////////////////////////////////////////////////////////////////////////////
	struct CrawlerIdleAction : public Action {
		CrawlerIdleAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerSetGoalAction : public Action {
		CrawlerSetGoalAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerSetTargetAction : public Action {
		CrawlerSetTargetAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerMoveAction : public Action {
		CrawlerMoveAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerRotateAction : public Action {
		CrawlerRotateAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerAttackAction : public Action {
		CrawlerAttackAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerWanderState : public State {
		const static size_t WANDER_ACTION_COUNT = 4;
		std::array<Action*, WANDER_ACTION_COUNT> action;
		const static size_t WANDER_ACTION_IDLE = 0;
		const static size_t WANDER_ACTION_SETGOAL = 1;
		const static size_t WANDER_ACTION_ROTATE = 2;
		const static size_t WANDER_ACTION_MOVE = 3;

		int curAction = WANDER_ACTION_IDLE;
		CrawlerWanderState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerWanderState();
		virtual void InitState() override final;

		virtual void Execute(float elapsedTime) override final;

		virtual float GetValue() override final;
	};

	struct CrawlerTraceState : public State {
		const static size_t TRACE_ACTION_COUNT = 3;
		std::array<Action*, TRACE_ACTION_COUNT> action;

		const static size_t TRACE_ACTION_SETGOAL = 0;
		const static size_t TRACE_ACTION_ROTATE = 1;
		const static size_t TRACE_ACTION_ATTACK = 2;

		int curAction = TRACE_ACTION_SETGOAL;

		CrawlerTraceState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerTraceState();

		virtual void InitState() override final;

		virtual void Execute(float elapsedTime) override final;

		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerStateManager {
		const static size_t STATE_WANDER = 0;
		const static size_t STATE_TRACE = 1;

		const static size_t STATE_COUNT = 2;
		std::array<State*, STATE_COUNT> state;
		CrawlerStateManager(SEnemyUnitComponent* comp);
		~CrawlerStateManager();

		void Init();
		void SetState();
		void Execute(float elapsedTime);

		SEnemyUnitComponent* enemyComp;
		int curState = 0;
		int GetCurState() const;
	};


	static struct CrawlerAnimIndex {
		const static UINT idle = 0U;
		const static UINT walk = 1U;
	};

	class DLL SEnemyCrawlerComponent : public SEnemyUnitComponent
	{
	protected:
		DirectX::XMFLOAT3							goal = { 0,0,0 };

		float										distance = 0;
		float										arriveTime = 0;
		float										moveTime = 0;

		DirectX::XMFLOAT2							angle;

		float										traceRange = 10;
		float										attackInterval = 2;
		float										attackTimer = 0;

		CrawlerStateManager* stateManager;

	public:
		void SetMoveTime(float t) { moveTime = t; }
		void MoveTimer(float elapsedTime) { moveTime += elapsedTime; }
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		bool SetGoal();
		bool RotateToGoal(float elapsedTime);
		bool MoveToGoal(float elapsedTime);
		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const DirectX::XMFLOAT3 pos) const;
		bool IsInTraceRange(const float distance) const;
		bool AttackTarget(float elapsedTime);
		// 공격 패킷을 어떻게 보내야 할까
		bool Idle(float elapsedTime);

		// virtual void PostAttack() override;
		virtual void Attack(SGameManagerComponent* gameManager) override;
		SEnemyCrawlerComponent();

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

		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyCrawlerComponent);
}