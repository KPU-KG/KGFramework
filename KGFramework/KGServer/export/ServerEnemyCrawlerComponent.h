#pragma once
#include "ServerBaseComponent.h"
#include "ServerEnemyUnitComponent.h"
#include "PhysicsComponent.h"
#include "Debug.h"
#include <vector>
#include <queue>
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

	struct CrawlerSetTargetAction : public Action {
		CrawlerSetTargetAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerShootAction : public Action {
		CrawlerShootAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};

	struct CrawlerSetAreaAction : public Action {
		CrawlerSetAreaAction(SEnemyUnitComponent* comp) : Action(comp) {}
		virtual bool Execute(float elapsedTime) override final;
		virtual void EndAction() override final;
	};


	// action[SHOOT_ACTION_SET_AREA] = new CrawlerSetAreaAction(enemyComp);
	// action[SHOOT_ACTION_ATTACK] = new CrawlerShootAction(enemyComp);

	//////////////////////////////////////////////////////////////////////////////
	// Enemy States
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerShootState : public State {
		const static size_t SHOOT_ACTION_COUNT = 2;
		std::array<Action*, SHOOT_ACTION_COUNT> action;
							
		bool isFinished = false;

		const static size_t SHOOT_ACTION_SET_AREA = 0;
		const static size_t SHOOT_ACTION_ATTACK = 1;

		int curAction = SHOOT_ACTION_SET_AREA;
		CrawlerShootState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerShootState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	// struct CrawlerChargeState : public State {
	// 
	// };

	struct CrawlerSetTargetState : public State {
		const static size_t SET_TARGET_ACTION_COUNT = 2;
		std::array<Action*, SET_TARGET_ACTION_COUNT> action;

		const static size_t SET_TARGET_ACTION_IDLE = 0;
		const static size_t SET_TARGET_ACTION_SET_TARGET = 1;

		bool isFinished = false;

		int curAction = SET_TARGET_ACTION_IDLE;
		CrawlerSetTargetState(SEnemyUnitComponent* comp) : State(comp) {}
		~CrawlerSetTargetState();
		virtual void InitState() override final;
		virtual void Execute(float elapsedTime) override final;
		virtual float GetValue() override final;
	};

	//////////////////////////////////////////////////////////////////////////////
	// Enemy State Manager
	//////////////////////////////////////////////////////////////////////////////

	struct CrawlerStateManager {
		const static size_t STATE_SET_TARGET = 0;
		const static size_t STATE_SHOOT_ATTACK = 1;

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

	class SCubeAreaRedComponent;

	class DLL SEnemyCrawlerComponent : public SEnemyUnitComponent
	{
	protected:
		static constexpr int maxHp = 10;
		float areaWidth = 3;
		std::queue<std::pair<float, float>>			shootArea;
		DirectX::XMFLOAT3							shootAreaCenter;
		DirectX::XMFLOAT3							shootTarget;

		DirectX::XMFLOAT2							angle;

		float										attackInterval = 0.5;
		float										attackTimer = 0;

		SCubeAreaRedComponent*		area;

		CrawlerStateManager* stateManager;
		bool inShootAction = false;
		bool isFilledArea = false;
	public:
		void SetIdleTime(float t) { idleTimer = t; }
		void IdleTimer(float elapsedTime) { idleTimer += elapsedTime; }
		void SetAttackTime(float t) { attackTimer = t; }
		void AttackTimer(float elapsedTime) { attackTimer += elapsedTime; }
		void ReadyNextAnimation(bool b) { changedAnimation = b; }

		void ChangeAnimation(const KG::Utill::HashString animId, UINT animIndex, UINT nextState, float blendingTime = 0.1f, int repeat = 1);
		float GetDistance2FromEnemy(DirectX::XMFLOAT3 pos) const;
		bool Idle(float elapsedTime);
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
		bool SetAttackArea();
		bool Shoot(float elapsedTime);

		virtual void Destroy() override;
	};

	REGISTER_COMPONENT_ID(SEnemyCrawlerComponent);
}